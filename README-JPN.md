# suim言語
suim言語は小さいプログラミング言語です。<br />
<br />
1. goto文は良いものです。<br />
2. 高速を求め続けます。<br />
3. suim.c suim.h のみで全てが記述されています。<br />
<br />
yoshihara_t@asterios.biz<br />
吉原　隆<br />
<br />
##suim言語を実行するまでの流れ
###test.suim (suim言語)

    GinInt = 0
    GclStr = "hoge"

    //a関数
    function start a
        inResult = 0
        if(GinInt== 0 || GclStr== "hoge")else ou85uzz7
            inResult = 1
            goto ui20c1zi
    @ou85uzz7
            inResult = 0
    
    @ui20c1zi
        return (inResult)
    function end

↓↓↓

###IL C# converter (中間言語コンバーター)

    string clBuffer = File.ReadAllText("test.suim");
    string clRefBuffer = "";
    int inRefVarCount = 0;
    int inRefOutCount = 0;
    Hashtable clRefTblValue = new Hashtable();
    ClsSuim.Convert("test", clBuffer, ref clRefBuffer, ref inRefVarCount, ref     inRefOutCount, ref clRefTblValue);

↓↓↓

###suimの中間言語

    --01\ns|aa:hoge\nf|a:6\n--\n1:ab=0\n2:ac=aa\n5:f:a\n6:ad=0\n7:i:(ab 0 == ac aa == ||)e11\n8:ad=1\n9:g:12\n11:ad=0\n14:o:ad\n14:r:\n15:e:\n--

↓↓↓

###suim.c や suim.dll で実行する
