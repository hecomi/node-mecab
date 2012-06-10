var mecab = require('./build/Release/mecab');
var str = 'いつもニコニコあなたの近くに這い寄る混沌ニャルラトホテプです！';

console.log(mecab.parse(str));
console.log(mecab.str2kana(str, false));
console.log(mecab.str2kana(str, true));

