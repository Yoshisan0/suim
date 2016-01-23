# suim language
suim is very tiny programming language.<br />
<br />
1. goto command is a good thing.<br />
2. It aims at high speed.<br />
3. this is written by just 2 files. (suim.h and suim.c)<br />
<br />
support@asterios.biz<br />
<br />
##How to use
###test.suim (suim language)

    GinInt = 0
    GclStr = "hoge"

    //a function
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

vvv

###IL C# converter (suim Intermediate language converter)

    string clBuffer = File.ReadAllText("test.suim");
    string clRefBuffer = "";
    int inRefVarCount = 0;
    int inRefOutCount = 0;
    Hashtable clRefTblValue = new Hashtable();
    ClsSuim.Convert("test", clBuffer, ref clRefBuffer, ref inRefVarCount, ref inRefOutCount, ref clRefTblValue);

vvv

###suim IL

    --01\ns|aa:hoge\nf|a:6\n--\n1:ab=0\n2:ac=aa\n5:f:a\n6:ad=0\n7:i:(ab 0 == ac aa == ||)e11\n8:ad=1\n9:g:12\n11:ad=0\n14:o:ad\n14:r:\n15:e:\n--

vvv

###execute on suim.c and suim.dll.
