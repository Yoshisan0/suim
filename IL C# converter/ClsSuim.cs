using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;

namespace PrjSuim
{
    public class ClsSuimLine
    {
        public int mLineNo1;    //コンパイル前のライン番号
        public int mLineNo2;    //コンパイル後のライン番号
        public string mLine;    //コンパイル前のライン文字列
        public bool mOutput;    //出力フラグ

        /// <summary>
        /// コンストラクタ
        /// </summary>
        /// <param name="clLine">ライン文字列</param>
        public ClsSuimLine(string clLine)
        {
            this.mLineNo1 = -1;
            this.mLineNo2 = -1;
            this.mLine = clLine;
            this.mOutput = true;
        }

        /// <summary>
        /// コンストラクタ
        /// </summary>
        /// <param name="inLineNo1">コンパイル前のライン番号</param>
        /// <param name="clLine">ライン文字列</param>
        public ClsSuimLine(int inLineNo1, string clLine)
        {
            this.mLineNo1 = inLineNo1;
            this.mLineNo2 = -1;
            this.mLine = clLine;
            this.mOutput = true;
        }

        /// <summary>
        /// コンストラクタ
        /// </summary>
        /// <param name="inLineNo1">コンパイル前のライン番号</param>
        /// <param name="inLineNo2">コンパイル後のライン番号</param>
        /// <param name="clLine">ライン文字列</param>
        public ClsSuimLine(int inLineNo1, int inLineNo2, string clLine)
        {
            this.mLineNo1 = inLineNo1;
            this.mLineNo2 = inLineNo2;
            this.mLine = clLine;
            this.mOutput = true;
        }
    }

    public class ClsSuim
    {
        private static string VERSION = "01";
        private static string ESCAPE_DQ = "`RnpZ4LW6HeIK736mTHX3eP9JHOE3bDvz`"; //エスケープされたダブルクォートの予約語
        private static string VAR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        public static string REGEX_NAME = "[a-zA-Z_][a-zA-Z0-9_]*";             //変数・関数名の正規表現
        public static string REGEX_CNG_NAME = "v_[a-f0-9]{32}";                 //変換後の文字列の正規表現

        /// <summary>
        /// 変数名取得
        /// </summary>
        /// <param name="inCount">カウンター</param>
        /// <returns>変数名</returns>
        public static string GetVarName(int inCount)
        {
            if (inCount >= ClsSuim.VAR.Length * ClsSuim.VAR.Length)
            {
                throw new SuimException("Overflow variable count. It must be less than " + (ClsSuim.VAR.Length * ClsSuim.VAR.Length) + ".");
            }

            //以下、十の位の設定
            int inIndex = (int)(inCount / ClsSuim.VAR.Length);
            string clName = ClsSuim.VAR[inIndex].ToString();

            //以下、一の位の設定
            inIndex = (int)(inCount % ClsSuim.VAR.Length);
            clName += ClsSuim.VAR[inIndex].ToString();

            return (clName);
        }

        /// <summary>
        /// 文字列内のすべての変数名をハッシュ値に変更する処理
        /// </summary>
        /// <param name="clTblVar2SmallVar">変数名格納テーブル</param>
        /// <param name="clTag">ヘッダータグ</param>
        /// <param name="clStr">文字列</param>
        /// <returns>変換後の文字列</returns>
        private static string GetMD5All(Hashtable clTblVar2SmallVar, string clTag, string clStr)
        {
            List<string> clList = new List<string>();

            MatchCollection clMatches = Regex.Matches(clStr, "(" + ClsSuim.REGEX_NAME + ")");
            foreach (Match clMatch in clMatches)
            {
                Match clMatchTmp = Regex.Match(clMatch.Groups[1].Value, ClsSuim.REGEX_CNG_NAME);
                if (clMatchTmp.Success) continue;

                clList.Add(clMatch.Groups[1].Value);
            }
            if(clList.Count== 0) return (clStr);

            //以下、ソート処理
            clList.Sort((a, b) => b.Length - a.Length);

            //以下、変換処理
            int inCnt, inMax = clList.Count;
            for (inCnt = 0; inCnt < inMax; inCnt++)
            {
                string clMD5 = ClsSuim.GetMD5(clTblVar2SmallVar, clTag, clList[inCnt]);
                clStr = clStr.Replace(clList[inCnt], clMD5);
            }

            return (clStr);
        }

        /// <summary>
        /// MD5のコードを取得
        /// </summary>
        /// <param name="clTblVar2SmallVar">変数名格納テーブル</param>
        /// <param name="clTag">ヘッダータグ</param>
        /// <param name="clStr">文字列</param>
        /// <returns>ハッシュコード</returns>
        private static string GetMD5(Hashtable clTblVar2SmallVar, string clTag, string clStr)
        {
            byte[] pchData = Encoding.UTF8.GetBytes(clStr);

            MD5CryptoServiceProvider clMD5 = new MD5CryptoServiceProvider();
            byte[] pchDataHash = clMD5.ComputeHash(pchData);
            clMD5.Clear();

            StringBuilder clResult = new StringBuilder();
            foreach (byte chData in pchDataHash)
            {
                clResult.Append(chData.ToString("x2"));
            }

            string clBuffer = clResult.ToString();
            clBuffer = clBuffer.ToLower();
            clBuffer = clTag + clBuffer;

            //以下、変数名格納処理
            if (clTblVar2SmallVar != null)
            {
                clTblVar2SmallVar[clStr] = clBuffer;
            }

            return (clBuffer);
        }

        /// <summary>
        /// 変数変換テーブル追加処理
        /// </summary>
        /// <param name="clTblCngVar">変数変換テーブル</param>
        /// <param name="clStr">変数名</param>
        /// <param name="inCntVar">カウンター</param>
        /// <returns>カウンター</returns>
        public static int AddCngVar(Hashtable clTblCngVar, string clStr, int inCntVar)
        {
            MatchCollection clMatches = Regex.Matches(clStr, "(" + ClsSuim.REGEX_CNG_NAME + ")");
            foreach (Match clMatch in clMatches)
            {
                string clVarName = clMatch.Groups[1].Value;

                bool isExist = clTblCngVar.ContainsKey(clVarName);
                if (isExist) continue;

                clTblCngVar[clVarName] = ClsSuim.GetVarName(inCntVar);
                inCntVar++;
            }

            return (inCntVar);
        }

        /// <summary>
        /// 代入演算子が２つ無いかチェック
        /// </summary>
        /// <param name="clLine">チェックする文字列</param>
        /// <returns>true:正常 false:２つ以上を検知</returns>
        private static bool ChkDoubleEqual(string clLine)
        {
            int inIndex1 = clLine.IndexOf('=');
            int inIndex2 = clLine.LastIndexOf('=');
            return (inIndex1 == inIndex2);
        }

        /// <summary>
        /// suim言語をc言語で解釈しやすい形式に変更
        /// </summary>
        /// <param name="clName">名前</param>
        /// <param name="clBuffer">suim言語バッファ</param>
        /// <param name="clRefBuffer">コンパイルされたsuim言語バッファ</param>
        /// <param name="inRefVarCount">変数カウンター</param>
        /// <param name="inRefOutCount">return値の最大数</param>
        public static void Convert(string clName, string clBuffer, ref string clRefBuffer, ref int inRefVarCount, ref int inRefOutCount, ref Hashtable clRefTblValue)
        {
            Hashtable clTblStr = new Hashtable();       //静的に定義された文字列テーブル
            Hashtable clTblFunc = new Hashtable();      //関数名テーブル（キーは関数名　値はClsSuimLine）
            Hashtable clTblLabel = new Hashtable();     //ラベルテーブル（キーはラベル名　値はClsSuimLine）
            Hashtable clTblCngVar = new Hashtable();    //変数名変換テーブル
            Hashtable clTblCallBack = new Hashtable();  //関数名テーブル
            Hashtable clTblVar2SmallVar = new Hashtable();
            int inVarCount = 0;
            int inOutMax = 0;

            clBuffer = clBuffer.Replace("\r\n", "\n");
            clBuffer = clBuffer.Replace("\\\"", ClsSuim.ESCAPE_DQ);
            string[] pclLines = clBuffer.Split('\n');

            int inCnt, inMax = pclLines.Length;
            for (inCnt = 0; inCnt < inMax; inCnt++)
            {
                Match clMatch;

                //以下、予約語チェック処理
                int inIndex = clBuffer.IndexOf(ClsSuim.ESCAPE_DQ);
                if (inIndex >= 0)
                {
                    throw new SuimException("[" + clName + ":" + inCnt + "] " + ClsSuim.ESCAPE_DQ + " is reserved word.");
                }

                clMatch = Regex.Match(pclLines[inCnt], ClsSuim.REGEX_CNG_NAME);
                if (clMatch.Success)
                {
                    throw new SuimException("[" + clName + ":" + inCnt + "] v_* is reserved word.");
                }

                //以下、文字列をhash値に変更する処理
                bool isHit;
                do
                {
                    isHit = false;

                    clMatch = Regex.Match(pclLines[inCnt], "(\".*?\")");
                    if (clMatch.Success)
                    {
                        string clValue = clMatch.Groups[1].Value;
                        string clMD5 = ClsSuim.GetMD5(null, "v_", clValue);

                        clValue = clValue.Replace(ClsSuim.ESCAPE_DQ, "\\\"");
                        clValue = clValue.Substring(1, clValue.Length - 2);
                        clTblStr[clMD5] = clValue;

                        string clStrSt = pclLines[inCnt].Substring(0, clMatch.Groups[1].Index);
                        string clStrEd = pclLines[inCnt].Substring(clMatch.Groups[1].Index + clMatch.Groups[1].Length);
                        pclLines[inCnt] = clStrSt + clMD5 + clStrEd;

                        inVarCount = ClsSuim.AddCngVar(clTblCngVar, clMD5, inVarCount); //変数変換テーブルに登録する処理

                        isHit = true;
                    }
                }
                while (isHit);

                //以下、コメントを削除する処理
                inIndex = pclLines[inCnt].IndexOf("//");
                if (inIndex >= 0)
                {
                    pclLines[inCnt] = pclLines[inCnt].Remove(inIndex);
                }

                //以下、トリミングする処理
                pclLines[inCnt] = pclLines[inCnt].Trim();
                if (string.IsNullOrEmpty(pclLines[inCnt])) continue;

                //以下、関数文法チェック処理
                inIndex = pclLines[inCnt].IndexOf("function");
                if (inIndex == 0)
                {
                    //以下、関数開始文法チェック処理
                    clMatch = Regex.Match(pclLines[inCnt], "^function start " + ClsSuim.REGEX_NAME + "$");
                    if (!clMatch.Success)
                    {
                        //以下、関数終了文法チェック処理
                        inIndex = pclLines[inCnt].IndexOf("function end");
                        if (inIndex != 0)
                        {
                            throw new SuimException("[" + clName + ":" + inCnt + "] Syntax error \"function\". " + pclLines[inCnt]);
                        }
                    }
                }

                //以下、ラベル文法チェック処理
                if (pclLines[inCnt][0] == '@')
                {
                    clMatch = Regex.Match(pclLines[inCnt], "^@" + ClsSuim.REGEX_NAME + "$");
                    if (!clMatch.Success)
                    {
                        throw new SuimException("[" + clName + ":" + inCnt + "] Label name error. " + pclLines[inCnt]);
                    }
                }

                //以下、goto文文法チェック処理
                inIndex = pclLines[inCnt].IndexOf("goto");
                if (inIndex == 0)
                {
                    clMatch = Regex.Match(pclLines[inCnt], "^goto " + ClsSuim.REGEX_NAME + "$");
                    if (!clMatch.Success)
                    {
                        throw new SuimException("[" + clName + ":" + inCnt + "] Goto error. " + pclLines[inCnt]);
                    }
                }

                //以下、if文文法チェック処理
                if (pclLines[inCnt][0] == 'i' && pclLines[inCnt][1] == 'f')
                {
                    clMatch = Regex.Match(pclLines[inCnt], "^if\\((.*)\\)else " + ClsSuim.REGEX_NAME + "$");
                    if (!clMatch.Success)
                    {
                        throw new SuimException("[" + clName + ":" + inCnt + "] Syntax error \"if\". " + pclLines[inCnt]);
                    }

                    string clStrCmp = clMatch.Groups[1].Value;
                    MatchCollection clMatches = Regex.Matches(clStrCmp, "(" + ClsSuim.REGEX_NAME + ")");
                    int inCount1 = clMatches.Count;
                    clStrCmp = clStrCmp.Replace("\t", "");
                    clStrCmp = clStrCmp.Replace(" ", "");
                    clStrCmp = clStrCmp.Replace("　", "");
                    clMatches = Regex.Matches(clStrCmp, "(" + ClsSuim.REGEX_NAME + ")");
                    int inCount2 = clMatches.Count;
                    if (inCount1 != inCount2)
                    {
                        throw new SuimException("[" + clName + ":" + inCnt + "] Syntax error. " + pclLines[inCnt]);
                    }
                }

                //以下、関数名だったらテーブルに追加しておく処理
                clMatch = Regex.Match(pclLines[inCnt], "function start (.*)");
                if (clMatch.Success)
                {
                    string clKey = clMatch.Groups[1].Value;
                    bool isExist = clTblFunc.ContainsKey(clKey);
                    if (!isExist)
                    {
                        clTblFunc[clKey] = true;
                    }
                    else
                    {
                        throw new SuimException("[" + clName + ":" + inCnt + "] Function duplication error. Function name " + clKey + ".");
                    }
                }

                //以下、ラベル名だったらテーブルに追加しておく処理
                clMatch = Regex.Match(pclLines[inCnt], "@(.*)");
                if (clMatch.Success)
                {
                    string clKey = clMatch.Groups[1].Value;
                    bool isExist = clTblLabel.ContainsKey(clKey);
                    if (!isExist)
                    {
                        clTblLabel[clKey] = true;
                    }
                    else
                    {
                        throw new SuimException("[" + clName + ":" + inCnt + "] Label duplication error. Label name " + clKey + ".");
                    }
                }

                //以下、行末の;チェック処理
                clMatch = Regex.Match(pclLines[inCnt], "^.*;$");
                if (clMatch.Success)
                {
                    throw new SuimException("[" + clName + ":" + inCnt + "] Can't analyze token ';'.");
                }

                //以下、不要な文字を削除する処理
                pclLines[inCnt] = pclLines[inCnt].Replace("\t", "");    //タブの削除
                pclLines[inCnt] = pclLines[inCnt].Replace(" ", "");     //スペースの削除
                pclLines[inCnt] = pclLines[inCnt].Replace("　", "");    //全角スペースの削除

                //以下、変換処理
                pclLines[inCnt] = pclLines[inCnt].Replace("functionstart", "f:");  //function
                pclLines[inCnt] = pclLines[inCnt].Replace("functionend", "e:");    //end
                pclLines[inCnt] = pclLines[inCnt].Replace("goto", "g:");            //goto
                clMatch = Regex.Match(pclLines[inCnt], "^if\\((.*?)\\)else(.*)$");
                if (clMatch.Success)
                {
                    pclLines[inCnt] = "i:" + pclLines[inCnt].Remove(0, 2);          //if
                }
                pclLines[inCnt] = pclLines[inCnt].Replace("return", "r:");          //return
                clMatch = Regex.Match(pclLines[inCnt], "^" + ClsSuim.REGEX_NAME + "\\(.*\\)$");
                if (clMatch.Success)
                {
                    pclLines[inCnt] = "c:" + pclLines[inCnt];                       //call
                }
                else
                {
                    clMatch = Regex.Match(pclLines[inCnt], "^([a-zA-Z0-9_,]*?)=(" + ClsSuim.REGEX_NAME + ")(\\(.*\\))$");
                    if (clMatch.Success)
                    {
                        string clValue1 = clMatch.Groups[1].Value;
                        string clValue2 = clMatch.Groups[2].Value;
                        string clValue3 = clMatch.Groups[3].Value;
                        pclLines[inCnt] = "c:" + clValue2 + clValue3 + " " + clValue1;
                    }
                }
            }

            //以下、ライン設定
            ArrayList clListLine = new ArrayList();
            inMax = pclLines.Length;
            for (inCnt = 0; inCnt < inMax; inCnt++)
            {
                string clLine = pclLines[inCnt];
                if (string.IsNullOrEmpty(clLine)) continue;

                ClsSuimLine clSuimLine = new ClsSuimLine(inCnt + 1, clLine);
                clListLine.Add(clSuimLine);
            }

            //以下、callのタプルを展開する処理
            while (true)
            {
                bool isLoop = false;

                inMax = clListLine.Count;
                for (inCnt = 0; inCnt < inMax; inCnt++)
                {
                    ClsSuimLine clSuimLine = clListLine[inCnt] as ClsSuimLine;
                    if (string.IsNullOrEmpty(clSuimLine.mLine)) continue;

                    if (!(clSuimLine.mLine[0] == 'c' && clSuimLine.mLine[1] == ':')) continue;

                    Match clMatch = Regex.Match(clSuimLine.mLine, "^c:(" + ClsSuim.REGEX_NAME + ")\\((.*)\\)$");
                    if (clMatch.Success)
                    {
                        string clFuncName = clMatch.Groups[1].Value;
                        bool isExist = clTblCallBack.ContainsKey(clFuncName);
                        if (!isExist) clTblCallBack[clFuncName] = true;
                        string clArgs = clMatch.Groups[2].Value;
                        if (!string.IsNullOrEmpty(clArgs))
                        {
                            string[] pclCells = clArgs.Split(',');

                            int inLop;
                            for (inLop = 0; inLop < pclCells.Length; inLop++)
                            {
                                string clArg = pclCells[inLop].ToLower();
                                if ("true".Equals(clArg) || "false".Equals(clArg))
                                {
                                    throw new SuimException("[" + clName + ":" + inCnt + "] Can't use boolean.");
                                }

                                string clLine = "o:" + pclCells[inLop];
                                ClsSuimLine clSuimLineTmp = new ClsSuimLine(clSuimLine.mLineNo1, clLine);
                                clListLine.Insert(inCnt + inLop, clSuimLineTmp);
                            }
                        }

                        clSuimLine.mLine = "c:" + clFuncName;
                    }
                    else
                    {
                        clMatch = Regex.Match(clSuimLine.mLine, "^c:(" + ClsSuim.REGEX_NAME + ")\\((.*)\\)\\s(.*)$");
                        if (clMatch.Success)
                        {
                            string clFuncName = clMatch.Groups[1].Value;
                            bool isExist = clTblCallBack.ContainsKey(clFuncName);
                            if (!isExist) clTblCallBack[clFuncName] = true;
                            string clArgs = clMatch.Groups[2].Value;
                            string clArgsIn = clMatch.Groups[3].Value;
                            if (!string.IsNullOrEmpty(clArgs))
                            {
                                string[] pclCells = clArgs.Split(',');

                                int inLop;
                                for (inLop = 0; inLop < pclCells.Length; inLop++)
                                {
                                    string clArg = pclCells[inLop].ToLower();
                                    if ("true".Equals(clArg) || "false".Equals(clArg))
                                    {
                                        throw new SuimException("[" + clName + ":" + inCnt + "] Can't use boolean.");
                                    }

                                    string clLine = "o:" + pclCells[inLop];
                                    ClsSuimLine clSuimLineTmp = new ClsSuimLine(clSuimLine.mLineNo1, clLine);
                                    clListLine.Insert(inCnt + inLop, clSuimLineTmp);
                                }
                            }

                            clSuimLine.mLine = "c:" + clFuncName + " " + clArgsIn;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    isLoop = true;
                    break;
                }

                if (!isLoop) break;
            }

            //以下、returnのタプルを展開する処理
            while (true)
            {
                bool isLoop = false;

                inMax = clListLine.Count;
                for (inCnt = 0; inCnt < inMax; inCnt++)
                {
                    ClsSuimLine clSuimLine = clListLine[inCnt] as ClsSuimLine;
                    if (string.IsNullOrEmpty(clSuimLine.mLine)) continue;

                    if (!(clSuimLine.mLine[0] == 'r' && clSuimLine.mLine[1] == ':')) continue;

                    Match clMatch = Regex.Match(clSuimLine.mLine, "^r:\\((.*)\\)$");
                    if (!clMatch.Success) continue;

                    string clArgs = clMatch.Groups[1].Value;
                    if (string.IsNullOrEmpty(clArgs)) continue;
                    string[] pclCells = clArgs.Split(',');

                    int inLop;
                    for(inLop= 0;inLop< pclCells.Length;inLop++) {
                        string clLine = "o:" + pclCells[inLop];
                        ClsSuimLine clSuimLineTmp = new ClsSuimLine(clSuimLine.mLineNo1, clLine);
                        clListLine.Insert(inCnt + inLop, clSuimLineTmp);
                    }

                    clSuimLine.mLine = "r:";

                    isLoop = true;
                    break;
                }

                if (!isLoop) break;
            }

            //以下、詳細な構文チェック処理
            int inOutCount = 0;
            inMax = clListLine.Count;
            for (inCnt = 0; inCnt < inMax; inCnt++)
            {
                ClsSuimLine clSuimLine = clListLine[inCnt] as ClsSuimLine;
                if (string.IsNullOrEmpty(clSuimLine.mLine)) continue;

                if (clSuimLine.mLine[0] == 'f' && clSuimLine.mLine[1] == ':') continue;
                if (clSuimLine.mLine[0] == 'e' && clSuimLine.mLine[1] == ':') continue;
                if (clSuimLine.mLine[0] == 'g' && clSuimLine.mLine[1] == ':') continue;
                if (clSuimLine.mLine[0] == '@') continue;

                Match clMatch;
                bool isHit;

                if (clSuimLine.mLine[0] == 'o' && clSuimLine.mLine[1] == ':')
                {
                    //以下、変数変換テーブルに登録する処理
                    clMatch = Regex.Match(clSuimLine.mLine, "^o:(.*)$");
                    if (clMatch.Success)
                    {
                        inOutCount++;
                        if (inOutCount > inOutMax) inOutMax = inOutCount;

                        string clStr = ClsSuim.GetMD5All(clTblVar2SmallVar, "v_", clMatch.Groups[1].Value);
                        inVarCount = ClsSuim.AddCngVar(clTblCngVar, clStr, inVarCount);

                        //以下、逆ポーランド記法に変換する処理
                        ClsContext clContext = new ClsContext(clStr, clName, clSuimLine.mLineNo1);
                        new ClsNodeTop(clContext);
                        clSuimLine.mLine = "o:" + clContext.GetContext();
                        continue;
                    }

                    throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Syntax error. " + clSuimLine.mLine);
                }

                if (clSuimLine.mLine[0] == 'i' && clSuimLine.mLine[1] == ':')
                {
                    clMatch = Regex.Match(clSuimLine.mLine, "^i:\\((.*?)\\)else(.*)$");
                    if (clMatch.Success)
                    {
                        string clStrCmp = ClsSuim.GetMD5All(clTblVar2SmallVar, "v_", clMatch.Groups[1].Value);
                        string clStr = clMatch.Groups[2].Value;
                        inVarCount = ClsSuim.AddCngVar(clTblCngVar, clStrCmp, inVarCount);

                        //以下、逆ポーランド記法に変換する処理
                        ClsContext clContext = new ClsContext(clStrCmp, clName, clSuimLine.mLineNo1);
                        new ClsNodeTop(clContext);
                        clSuimLine.mLine = "i:(" + clContext.GetContext() + ")e" + clStr;
                        continue;
                    }

                    throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Syntax error. " + clSuimLine.mLine);
                }

                if (clSuimLine.mLine[0] == 'c' && clSuimLine.mLine[1] == ':')
                {
                    clMatch = Regex.Match(clSuimLine.mLine, "^c:(" + ClsSuim.REGEX_NAME + ")$");
                    if (clMatch.Success)
                    {
                        inOutCount = 0;
                        continue;
                    }
                    else
                    {
                        clMatch = Regex.Match(clSuimLine.mLine, "^c:(" + ClsSuim.REGEX_NAME + ") (.*)$");
                        if (clMatch.Success)
                        {
                            string clFuncName = clMatch.Groups[1].Value;
                            string clArgsIn = clMatch.Groups[2].Value;
                            string[] pclCells = clArgsIn.Split(',');

                            clArgsIn = "";
                            int inLop, inMaxLop = pclCells.Length;
                            for (inLop = 0; inLop < inMaxLop; inLop++)
                            {
                                string clInArg = pclCells[inLop];
                                if (string.IsNullOrEmpty(clInArg)) continue;

                                string clVerName = ClsSuim.GetMD5(clTblVar2SmallVar, "v_", clInArg);
                                inVarCount = ClsSuim.AddCngVar(clTblCngVar, clVerName, inVarCount); //変数変換テーブルに登録する処理

                                clArgsIn += clVerName + ",";
                            }
                            if (clArgsIn.Length>= 1)
                            {
                                clArgsIn = clArgsIn.Substring(0, clArgsIn.Length - 1);
                            }

                            clSuimLine.mLine = "c:" + clFuncName + " " + clArgsIn;

                            inOutCount = 0;
                            continue;
                        }
                    }

                    throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Syntax error. " + clSuimLine.mLine);
                }

                if (clSuimLine.mLine[0] == 'r' && clSuimLine.mLine[1] == ':')
                {
                    clMatch = Regex.Match(clSuimLine.mLine, "^r:$");
                    if (clMatch.Success)
                    {
                        inOutCount = 0;
                        continue;
                    }

                    throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Syntax error. " + clSuimLine.mLine);
                }

                //以下、二重の代入演算子チェック
                isHit = ClsSuim.ChkDoubleEqual(clSuimLine.mLine);
                if (!isHit)
                {
                    throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Syntax error. " + clSuimLine.mLine);
                }

                //以下、代入文法チェック
                clMatch = Regex.Match(clSuimLine.mLine, "^(" + ClsSuim.REGEX_NAME + ")=(.*)$");
                if (clMatch.Success)
                {
                    //以下、変数変換テーブルに登録する処理
                    string clStrVal = ClsSuim.GetMD5(clTblVar2SmallVar, "v_", clMatch.Groups[1].Value);
                    string clStr = ClsSuim.GetMD5All(clTblVar2SmallVar, "v_", clMatch.Groups[2].Value);
                    inVarCount = ClsSuim.AddCngVar(clTblCngVar, clStrVal, inVarCount);
                    inVarCount = ClsSuim.AddCngVar(clTblCngVar, clStr, inVarCount);

                    //以下、逆ポーランド記法に変換する処理
                    ClsContext clContext = new ClsContext(clStr, clName, clSuimLine.mLineNo1);
                    new ClsNodeTop(clContext);
                    clSuimLine.mLine = clStrVal + "=" + clContext.GetContext();
                    continue;
                }

                throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Syntax error. " + clSuimLine.mLine);
            }

            //以下、ラベル存在チェック処理
            inMax = clListLine.Count;
            for (inCnt = 0; inCnt < inMax; inCnt++)
            {
                ClsSuimLine clSuimLine = clListLine[inCnt] as ClsSuimLine;

                Match clMatch = Regex.Match(clSuimLine.mLine, "^g:(.*)$");
                if (clMatch.Success)
                {
                    bool isExist = clTblLabel.ContainsKey(clMatch.Groups[1].Value);
                    if (!isExist)
                    {
                        throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Can't find label. Label name " + clMatch.Groups[1].Value + ".");
                    }
                }

                clMatch = Regex.Match(clSuimLine.mLine, "^i:\\(.*\\)e(.*)$");
                if (clMatch.Success)
                {
                    bool isExist = clTblLabel.ContainsKey(clMatch.Groups[1].Value);
                    if (!isExist)
                    {
                        throw new SuimException("[" + clName + ":" + clSuimLine.mLineNo1 + "] Can't find label. Label name " + clMatch.Groups[1].Value + ".");
                    }
                }
            }

            //以下、行番号を追加する処理
            foreach (ClsSuimLine clSuimLine in clListLine)
            {
                clSuimLine.mLine = clSuimLine.mLineNo1 + ":" + clSuimLine.mLine;
            }

            //以下、各定義を冒頭に追加する処理
            int inLineNo = 0;
            {
                //以下、ヘッダー
                ClsSuimLine clSuimLine = new ClsSuimLine("--" + ClsSuim.VERSION);
                clListLine.Insert(inLineNo, clSuimLine);
                inLineNo++;

                //以下、各定義を追加
                foreach (string clKey in clTblStr.Keys)
                {
                    string clValue = clTblStr[clKey] as string;
                    clSuimLine = new ClsSuimLine("s|" + clKey + ":" + clValue);
                    clListLine.Insert(inLineNo, clSuimLine);
                    inLineNo++;
                }

                foreach (string clKey in clTblFunc.Keys)
                {
                    clSuimLine = new ClsSuimLine("f|" + clKey + ":");
                    clListLine.Insert(inLineNo, clSuimLine);
                    inLineNo++;
                }

                //以下、セパレーター
                clSuimLine = new ClsSuimLine("--");
                clListLine.Insert(inLineNo, clSuimLine);

                //以下、テーブルにジャンプ先を設定しておく処理
                foreach (ClsSuimLine clSuimLineTmp in clListLine)
                {
                    Match clMatch = Regex.Match(clSuimLineTmp.mLine, "f:(.*)");
                    if (clMatch.Success)
                    {
                        string clFuncName = clMatch.Groups[1].Value;
                        clTblFunc[clFuncName] = clSuimLineTmp;
                    }

                    clMatch = Regex.Match(clSuimLineTmp.mLine, "@(.*)");
                    if (clMatch.Success)
                    {
                        string clLabelName = clMatch.Groups[1].Value;
                        clTblLabel[clLabelName] = clSuimLineTmp;

                        clSuimLineTmp.mOutput = false;  //ラベルは出力しない
                    }
                }

                //以下、終端
                clSuimLine = new ClsSuimLine("--");
                clListLine.Add(clSuimLine);
            }

            //以下、コンパイル後のライン番号を設定する処理
            inLineNo = 0;
            foreach (ClsSuimLine clSuimLine in clListLine)
            {
                clSuimLine.mLineNo2 = inLineNo;
                if (!clSuimLine.mOutput) continue;
                inLineNo++;
            }

            //以下、ジャンプ先の行番号設定処理
            foreach (ClsSuimLine clSuimLine in clListLine)
            {
                if (!clSuimLine.mOutput) continue;

                Match clMatch = Regex.Match(clSuimLine.mLine, "^f\\|(.*):$");
                if (clMatch.Success)
                {
                    string clFuncName = clMatch.Groups[1].Value;
                    ClsSuimLine clSuimLineTmp = clTblFunc[clFuncName] as ClsSuimLine;
                    clSuimLine.mLine = "f|" + clFuncName + ":" + clSuimLineTmp.mLineNo2;
                    continue;
                }

                clMatch = Regex.Match(clSuimLine.mLine, "^(\\d*?):i:\\((.*)\\)e(.*)$");
                if (clMatch.Success)
                {
                    string clLineNo = clMatch.Groups[1].Value;
                    string clStrCmp = clMatch.Groups[2].Value;
                    string clLabelName = clMatch.Groups[3].Value;
                    ClsSuimLine clSuimLineTmp = clTblLabel[clLabelName] as ClsSuimLine;
                    clSuimLine.mLine = clLineNo + ":i:(" + clStrCmp + ")e" + clSuimLineTmp.mLineNo2;
                    continue;
                }

                clMatch = Regex.Match(clSuimLine.mLine, "^(\\d*?):g:(.*)$");
                if (clMatch.Success)
                {
                    string clLineNo = clMatch.Groups[1].Value;
                    string clLabelName = clMatch.Groups[2].Value;
                    ClsSuimLine clSuimLineTmp = clTblLabel[clLabelName] as ClsSuimLine;
                    clSuimLine.mLine = clLineNo + ":g:" + clSuimLineTmp.mLineNo2;
                    continue;
                }
            }

            //以下、変数名を２文字にする処理
            foreach (ClsSuimLine clSuimLine in clListLine)
            {
                foreach (string clKey in clTblCngVar.Keys)
                {
                    string clValue = clTblCngVar[clKey] as string;
                    clSuimLine.mLine = clSuimLine.mLine.Replace(clKey, clValue);
                }
            }

            //以下、各ラインを合成する処理
            StringBuilder clBufferBuild = new StringBuilder();
            foreach (ClsSuimLine clSuimLine in clListLine)
            {
                if (!clSuimLine.mOutput) continue;
                clBufferBuild.Append(clSuimLine.mLine + "\n");
            }
            string clBufferRes = clBufferBuild.ToString();
            clBufferRes = clBufferRes.Trim();

            //以下、出力設定
            clRefBuffer = string.Copy(clBufferRes);
            inRefVarCount = inVarCount;
            inRefOutCount = inOutMax;

            clRefTblValue = new Hashtable();
            foreach (string clKey in clTblVar2SmallVar.Keys)
            {
                string clValue = clTblVar2SmallVar[clKey] as string;

                bool isExist = clTblCngVar.ContainsKey(clValue);
                if (!isExist) continue;

                clRefTblValue[clKey] = clTblCngVar[clValue] as string;
            }
        }
    }

    public class SuimException : Exception
    {
        public SuimException(string clErrMsg)
            : base(clErrMsg)
        {
        }
    }
}
