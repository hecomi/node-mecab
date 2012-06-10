#include <boost/noncopyable.hpp>
#include <boost/tokenizer.hpp>
#include <mecab.h>
#include <node.h>

using namespace v8;

/**
 * MeCab::Taggerを吐き出すだけのクラス
 */
class MeCabTagger : private boost::noncopyable
{
public:
	static MeCab::Tagger* getInstance() {
		static MeCab::Tagger *tagger = MeCab::createTagger("");
		return tagger;
	}
};

/**
 * MeCab に文字列を突っ込んで解析させる
 * @param[in] args JavaScript側から渡される引数
 * @return 正常な場合は MeCab で解析した結果（2次元配列）、ダメな場合は undefined
 */
Handle<Value> parse(const Arguments& args)
{
	HandleScope scope;

	// 引数が文字列かどうかチェック
	if (!args[0]->IsString()) {
		Local<String> msg = String::New("[MeCab] Error! Argument of 'parse' must be String.");
		ThrowException(Exception::TypeError(msg));
		return scope.Close(Undefined());
	}
	String::Utf8Value str(args[0]);

	// Tagger を取得して解析結果の node を得る
	auto tagger = MeCabTagger::getInstance();
	auto node   = tagger->parseToNode(*str);

	// node の数を調べる
	int num = 0;
	for (auto node2 = node; node2->next; node2 = node2->next, ++num) ;
	Local<Array> result = Array::New(num);

	// 解析結果を配列に格納
	int i = 0;
	for (node = node->next; node->next; node = node->next) {
		boost::char_separator<char> sep(",");
		boost::tokenizer<boost::char_separator<char>> tok(std::string(node->feature), sep);
		Local<Array> node_result = Array::New(12);
		int j = 0;
		for (auto it = tok.begin(); it != tok.end(); ++it) {
			node_result->Set( Number::New(j), String::New( (*it).c_str() ) );
			++j;
		}
		result->Set(Number::New(i), node_result);
		++i;
	}

	return scope.Close(result);
}

/**
 * 文字列をカタカナに変換する
 * @param[in] args JavaScript側から渡される引数
 * @return 正常な場合はカタカナになった文字列、ダメな場合は undefined
 */
Handle<Value> str2kana(const Arguments& args)
{
	HandleScope scope;

	// 引数が文字列かどうかチェック
	if (!args[0]->IsString()) {
		Local<String> msg = String::New("[MeCab] Error! Argument of 'parse' must be String.");
		ThrowException(Exception::TypeError(msg));
		return scope.Close(Undefined());
	}
	String::Utf8Value str(args[0]);

	// Tagger を取得して解析結果の node を得る
	auto tagger = MeCabTagger::getInstance();
	auto node   = tagger->parseToNode(*str);

	// 解析結果を文字列として連結
	std::string result;
	for (node = node->next; node->next; node = node->next) {
		boost::char_separator<char> sep(",");
		boost::tokenizer<boost::char_separator<char>> tok(std::string(node->feature), sep);
		int i = 0;
		for (auto it = tok.begin(); it != tok.end(); ++it) {
			if (i == 8) result += *it;
			++i;
		}
		// 辞書に登録されていないものがやってきたときは↑で追加されないので
		// 第2引数を true にした場合は元々の文字列を追加するようにする
		if (i < 8 && args[1]->BooleanValue()) {
			std::string surface = node->surface;
			result += std::string(surface.begin(), surface.begin() + node->length);
		}
	}

	return scope.Close(String::New(result.c_str()));
}

/**
 * Node.js の世界へいってらっしゃい
 */
void init(Handle<Object> target) {
	HandleScope scope;
	target->Set(
		String::NewSymbol("parse"),
		FunctionTemplate::New(parse)->GetFunction()
	);
	target->Set(
		String::NewSymbol("str2kana"),
		FunctionTemplate::New(str2kana)->GetFunction()
	);
}

NODE_MODULE(mecab, init)

