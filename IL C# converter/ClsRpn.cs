using System; 
using System.Net;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace PrjSuim
{
    public class ClsRpn
    {
        private List<string> mTokens;

        public IEnumerable<string> Tokens
        {
            get { return mTokens; }
            private set { mTokens.AddRange(value); }
        }

        public ClsRpn()
        {
            mTokens = new List<string>();
        }

        public void Add(string token)
        {
            mTokens.Add(token);
        }

        private ClsRpn(IEnumerable<string> tokens)
        {
            Tokens = tokens;
        }

        public static ClsRpn CreateFromExp(string exp)
        {
            var tokens = exp.Split(' ').Where(s => s != "").ToList();
            return new ClsRpn(tokens);
        }

        public override string ToString()
        {
            return String.Join(" ", Tokens.ToArray());
        }
    }

    public abstract class ClsNode
    {
        public abstract void Parse(ClsContext clContext);
        public static bool IsOperator(char c)
        {
            return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%');
        }

        public static bool IsParenthesis(char c)
        {
            return (c == '(' || c == ')');
        }

        public static bool IsSign(char c)
        {
            return (c == '-' || c == '+');
        }

        public static int GetSign(string s)
        {
            if (s[0] == '-')
                return -1;
            return 1;
        }
    }

    public class ClsNodeSignedNumber : ClsNode
    {
        public override void Parse(ClsContext clContext)
        {
            var clToken = clContext.CurrentToken;
            Match clMatch = Regex.Match(clToken, ClsSuim.REGEX_CNG_NAME);
            if (!clMatch.Success)
            {
                int inSign = 1;
                string clSign = "";
//              IEnumerable<char> nstr = clToken;
                if (IsSign(clToken[0]))
                {
                    inSign = GetSign(clToken);
                    clSign = clToken;
                    clToken = clContext.NextToken();
                }

                clMatch = Regex.Match(clToken, ClsSuim.REGEX_CNG_NAME);
                if (clMatch.Success)
                {
                    clToken = clSign + clToken;
                }
                else
                {
                    clToken = string.Format("{0}", inSign * Convert.ToDecimal(clToken));
                }
            }
            clContext.Notation.Add(clToken);
            clContext.NextToken();
        }
    }

    public class ClsNodeFactor : ClsNode
    {
        public override void Parse(ClsContext clContext)
        {
            var clToken = clContext.CurrentToken;
            if (clToken == "(")
            {
                clContext.SkipToken("(");
                ClsNode clNode = new ClsNodeAdd();
                clNode.Parse(clContext);
                clContext.SkipToken(")");
            }
            else
            {
                ClsNode clNode = new ClsNodeSignedNumber();
                clNode.Parse(clContext);
            }
        }
    }

    public class ClsNodeMultiply : ClsNode
    {
        public override void Parse(ClsContext clContext)
        {
            ClsNode clNode1 = new ClsNodeFactor();
            clNode1.Parse(clContext);
            string clToken = clContext.CurrentToken;
            while (clToken == "*" || clToken == "/" || clToken == "%")
            {
                clContext.NextToken();
                ClsNode clNode2 = new ClsNodeFactor();
                clNode2.Parse(clContext);
                clContext.Notation.Add(clToken);
                clToken = clContext.CurrentToken;
            }
        }
    }

    public class ClsNodeAdd : ClsNode
    {
        public override void Parse(ClsContext clContext)
        {
            var clToken = clContext.CurrentToken;
            ClsNode clNode1 = new ClsNodeMultiply();
            clNode1.Parse(clContext);
            clToken = clContext.CurrentToken;
            while (clToken == "+" || clToken == "-")
            {
                clContext.NextToken();
                ClsNode clNode2 = new ClsNodeMultiply();
                clNode2.Parse(clContext);
                clContext.Notation.Add(clToken);
                clToken = clContext.CurrentToken;
            }
        }
    }

    public class ClsNodeThan : ClsNode
    {
        public override void Parse(ClsContext clContext)
        {
            var clToken = clContext.CurrentToken;
            ClsNode clNode1 = new ClsNodeAdd();
            clNode1.Parse(clContext);
            clToken = clContext.CurrentToken;
            while (clToken == "≦" || clToken == "≧" || clToken == "<" || clToken == ">")
            {
                clContext.NextToken();
                ClsNode clNode2 = new ClsNodeAdd();
                clNode2.Parse(clContext);
                clContext.Notation.Add(clToken);
                clToken = clContext.CurrentToken;
            }
        }
    }

    public class ClsNodeEqual : ClsNode
    {
        public override void Parse(ClsContext clContext)
        {
            var clToken = clContext.CurrentToken;
            ClsNode clNode1 = new ClsNodeThan();
            clNode1.Parse(clContext);
            clToken = clContext.CurrentToken;
            while (clToken == "＝" || clToken == "≠")
            {
                clContext.NextToken();
                ClsNode clNode2 = new ClsNodeThan();
                clNode2.Parse(clContext);
                clContext.Notation.Add(clToken);
                clToken = clContext.CurrentToken;
            }
        }
    }

    public class ClsNodeTerms : ClsNode
    {
        public override void Parse(ClsContext clContext)
        {
            var clToken = clContext.CurrentToken;
            ClsNode clNode1 = new ClsNodeEqual();
            clNode1.Parse(clContext);
            clToken = clContext.CurrentToken;
            while (clToken == "＆" || clToken == "｜")
            {
                clContext.NextToken();
                ClsNode clNode2 = new ClsNodeEqual();
                clNode2.Parse(clContext);
                clContext.Notation.Add(clToken);
                clToken = clContext.CurrentToken;
            }
        }
    }

    public class ClsNodeTop : ClsNode
    {
        /// <summary>
        /// コンストラクタ
        /// </summary>
        /// <param name="clContext">テキスト管理クラス</param>
        public ClsNodeTop(ClsContext clContext)
        {
            this.Parse(clContext);
        }

        public override void Parse(ClsContext clContext)
        {
            ClsNode clNode1 = new ClsNodeTerms();
            clNode1.Parse(clContext);
        }
    }

    public class ClsTokenizer
    {
        private string mName;
        private int mLineNo;
        private string mExp;
        private int mIx = 0;
        private IEnumerator<string> mIte;

        public ClsTokenizer(string clExp, string clName, int inLineNo)
        {
            this.mName = clName;
            this.mLineNo = inLineNo;

            this.mExp = clExp;
            this.mIte = this.GetTokens().GetEnumerator();
        }

        public string NextToken()
        {
            if (this.mIte.MoveNext())
            {
                return this.mIte.Current;
            }
            return null;
        }

        public string Current
        {
            get { return this.mIte.Current; }
        }

        public void SkipToken(string s)
        {
            if (Current != s)
                throw new SuimException("[" + this.mName + "," + this.mLineNo + "] Syntax error.");
            this.NextToken();
        }

        public IEnumerable<string> GetTokens()
        {
            char chChr = this.NextChar();
            string clToken = "";
            while (chChr != (char)0)
            {
                if (chChr == 'v')
                {
                    int inCnt;
                    for (inCnt = 0; inCnt < 2 + 32; inCnt++)
                    {
                        clToken += chChr;
                        chChr = this.NextChar();
                    }
                    continue;
                }
                else if (char.IsDigit(chChr) || chChr == '.')
                {
                    clToken += chChr;
                }
                else
                {
                    if (clToken != "")
                        yield return clToken;
                    clToken = "";
                    if (ClsTokenizer.IsSymbol(chChr))
                        yield return chChr.ToString();
                    else if (chChr != ' ')
                        throw new SuimException("[" + this.mName + "," + this.mLineNo + "] Syntax error.");
                }
                chChr = NextChar();
            }
            if (clToken != "")
                yield return clToken;
        }

        public static bool IsSymbol(char c)
        {
            if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
                c == '＝' || c == '≠' || c == '≦' || c == '≧' || c == '<' || c == '>' || c == '＆' || c == '｜' ||
                c == '(' || c == ')')
                return true;
            return false;
        }

        public static bool IsSymbol(string s)
        {
            if (s.Length == 1)
            {
                if (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/' || s[0] == '%' ||
                    s[0] == '＝' || s[0] == '≠' || s[0] == '≦' || s[0] == '≧' || s[0] == '<' || s[0] == '>' || s[0] == '＆' || s[0] == '｜' ||
                    s[0] == '(' || s[0] == ')')
                    return true;
            }
            return false;
        }

        private char NextChar()
        {
            return _NextChar();
        }

        private char _NextChar()
        {
            if (mIx < mExp.Length)
                return mExp[mIx++];
            return (char)0;

        }
    }

    public class ClsContext
    {
        public ClsTokenizer Tokenizer { get; set; }
        public ClsRpn Notation { get; set; }

        /// <summary>
        /// コンストラクタ
        /// </summary>
        /// <param name="clExp">式</param>
        /// <param name="clName">名前</param>
        /// <param name="inLineNo">行番号</param>
        public ClsContext(string clExp, string clName, int inLineNo)
        {
            clExp = clExp.Replace("==", "＝");
            clExp = clExp.Replace("<=", "≦");
            clExp = clExp.Replace(">=", "≧");
            clExp = clExp.Replace("!=", "≠");
            clExp = clExp.Replace("&&", "＆");
            clExp = clExp.Replace("||", "｜");

            Tokenizer = new ClsTokenizer(clExp, clName, inLineNo);
            Tokenizer.NextToken();
            Notation = new ClsRpn();
        }

        public string GetContext()
        {
            string clExp = "" + this.Notation;
            clExp = clExp.Replace("＝", "==");
            clExp = clExp.Replace("≦", "<=");
            clExp = clExp.Replace("≧", ">=");
            clExp = clExp.Replace("≠", "!=");
            clExp = clExp.Replace("＆", "&&");
            clExp = clExp.Replace("｜", "||");

            return (clExp);
        }

        public string NextToken()
        {
            if (Tokenizer.NextToken() == null)
                return null;
            return CurrentToken;
        }

        public string CurrentToken
        {
            get { return Tokenizer.Current; }
        }

        public void SkipToken(string token)
        {
            Tokenizer.SkipToken(token);
        }
    }
}
