# suim
tiny programming language.



suim����͏������v���O���~���O����ł��B

1.goto���͗ǂ����̂��B
2.���������ߑ�����B
3.suim.c suim.h �݂̂őS�Ă��L�q����Ă���B

yoshihara_t@asterios.biz
�g���@��

----------------------------------------------------------------
test.suim (suim����)
--------------------------------

GinInt = 0
GclStr = "hoge"

//a�֐�
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

--------------------------------

������

--------------------------------

IL C# converter (���Ԍ���R���o�[�^�[)

string clBuffer = File.ReadAllText("test.suim");
string clRefBuffer = "";
int inRefVarCount = 0;
int inRefOutCount = 0;
Hashtable clRefTblValue = new Hashtable();
ClsSuim.Convert("test", clBuffer, ref clRefBuffer, ref inRefVarCount, ref inRefOutCount, ref clRefTblValue);

--------------------------------

������

--------------------------------

suim�̒��Ԍ���

--01\ns|aa:hoge\nf|a:6\n--\n1:ab=0\n2:ac=aa\n5:f:a\n6:ad=0\n7:i:(ab 0 == ac aa == ||)e11\n8:ad=1\n9:g:12\n11:ad=0\n14:o:ad\n14:r:\n15:e:\n--

----------------------------------------------------------------
