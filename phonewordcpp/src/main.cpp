#include <string.h>
#include <ctype.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <list>
#include <iterator>
#include <unordered_map>
#include <memory>
#include <assert.h>

#ifdef TIME_IT
#include <sys/time.h>
#endif

#ifdef _UNICODE
#define _T(text) L##text
#else
#define _T(text) text
#endif
namespace jz{
#ifdef _UNICODE
typedef wchar_t Char;
typedef ::std::wstring String;
typedef ::std::wifstream Ifstream;
typedef ::std::wofstream Ofstream;
typedef ::std::wstringstream Stringstream;
typedef ::std::wostream Ostream;
#define Cout ::std::wcout
#else
typedef char Char;
typedef ::std::string String;
typedef ::std::ifstream Ifstream;
typedef ::std::ofstream Ofstream;
typedef ::std::stringstream Stringstream;
typedef ::std::ostream Ostream;
#define Cout std::cout
#endif


typedef std::list<String> StringList;
typedef std::unordered_map<Char, Char> CharCharMap;
typedef std::unordered_map<Char, String> CharStringMap;
typedef std::unordered_map<String, std::list<String> > StringStringListMap;

template <typename T>
class Matrix {
    std::vector<T> data;

    Matrix(const Matrix&){}
    Matrix& operator=(const Matrix&){}
public:
    typedef Matrix<T> ThisType;
    const int NROW, NCOL, SIZE;

    Matrix(std::size_t nrow, std::size_t ncol)
        : NROW(nrow), NCOL(ncol), SIZE(nrow*ncol), data(nrow*ncol)
    {}
    inline std::size_t size() const {
        return SIZE;
    }
    inline T& operator[](std::size_t i) {
        assert(i<SIZE);
        return data[i];
    }
    T& operator()(std::size_t i, std::size_t j) {
        std::size_t k = i*NCOL +j;
        assert(k<SIZE);
        return (*this)[k];
    }
    const T& operator()(std::size_t i, std::size_t j) const{
        ThisType& me = const_cast<ThisType&>(*this);
        return me(i,j);
    }
};

typedef Matrix<StringList> StringListMatrix;

#ifdef TIME_IT
long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
#endif

struct PhoneNumberWord {
    enum { MAX_LINE_LEN = 128, MIN_WORD_LEN = 2 };
    int minWordLen;

    PhoneNumberWord(): minWordLen(MIN_WORD_LEN) {
        //
        CharStringMap d2a;
        d2a[_T('2')] = _T("ABC");
        d2a[_T('3')] = _T("DEF");
        d2a[_T('4')] = _T("GHI");
        d2a[_T('5')] = _T("JKL");
        d2a[_T('6')] = _T("NMO");
        d2a[_T('7')] = _T("PQRS");
        d2a[_T('8')] = _T("TUV");
        d2a[_T('9')] = _T("WXYZ");

        for(CharStringMap::iterator it=d2a.begin(); it!=d2a.end(); ++it) {
            String& w=it->second;
            Char d = it->first;
            for(String::iterator itc=w.begin(); itc!=w.end(); ++itc) {
                a2d[*itc] = d;
            }
        }
    }

    void processDic() {
        for(StringList::iterator it=words.begin(); it!= words.end(); ++it) {
            String& w(*it);
            String number;
            int n = 0;
            for(String::iterator itc=w.begin(); itc!=w.end(); ++itc) {
                CharCharMap::iterator itd = a2d.find(*itc);
                if( itd == a2d.end() ) { // unknown letter
                    n = 0;
                    break;
                }
               number += itd->second;
            }
            if( number.length() > 1) {
                n2w[number].push_back( w );
            }
        }
    }

    void setMinWordLength(int len) {
        minWordLen = len;
    }

    bool loadDict(const char *filename = "/usr/share/dict/words") {
        Ifstream file(filename);
        if( !file.is_open() ) return false;
        Char buf[MAX_LINE_LEN];
        int nline = 0;
        while( file.getline(buf, MAX_LINE_LEN) ) {
            String line(buf);
            String s;
            bool validWord = true;
            for(String::iterator it=line.begin(); it!=line.end(); ++it) {
                if( std::isalpha(*it) ) {
                    s += std::toupper(*it);
                }else if( *it == '\'' || *it=='-' ) {
                    break;
                }else{
                    validWord = false;
                    break;
                }
            }
            if( validWord && s.length() >= minWordLen ) {
                words.push_back(s);
            }
            ++nline;
        }
        processDic();
        return true;
    }

    // dynamic programming to store matched words
    //                     Matched String Matrix
    //             _____________ startPos ___________________
    //             |         0            1        2        3
    //             | 2       AD
    // wordLength  | 3       BOB,BIZ
    //             | 4
    //
    void findWord(String adigits, Ostream& os) const {
        String digits;
        for(int i=0; i< adigits.length(); ++i) // ignore all non-digits
            if( isdigit(adigits[i]) )
                digits += adigits[i];
        const size_t N = digits.length();
        if( N == 0) {
            os << "No digits in " << adigits << std::endl;
            return ;
        }
        StringListMatrix m(N, N);
        for(int i=0; i<N-1; ++i) {  // scan
            if( isSep(digits[i]) ) continue;
            if( isSep(digits[i+1]) ) {
                ++i;
                continue;
            }

            for(int j=i+minWordLen; j<=N; ++j) { // end of string
                matchWord(digits, i, j-i, m);
                if( isSep(digits[j]) ) {  // separator
                    break;
                }
            }
        }
        // fill the matchedowrds
//        printMatrix(m, os);
        StringList sl;
        printWords(digits, m, sl);
        std::ostream_iterator<String> outit(os, "\n");
        std::copy(sl.begin(), sl.end(), outit);
    }
    static bool isSep(Char c) {
        return !isdigit(c) || c == _T('1') || c == _T('0');
    }


    // return count ofmatched words
    void matchWord(const String& num, int startpos, int length, StringListMatrix& matchedWords) const {
        StringStringListMap::const_iterator it = n2w.find(num);
        if( it != n2w.end() )
            matchedWords(length, startpos) = it->second;
    }
    void printMatrix( StringListMatrix& m, Ostream& os ) {
        os << "<startPos, length: matched Strings>" << std::endl;
        for(int i=minWordLen; i<m.NROW; ++i) {
            for(int j=0; j<m.NCOL; ++j) {
                if( m(i,j).size() > 0 ) {
                    os << "<" << j << "," << i << ":";
                    for(StringList::iterator it=m(i,j).begin(); it != m(i,j).end(); ++it) {
                        os << *it << " ";
                    }
                    os << ">" << std::endl;
                }
            }
        }
    }

    void printWords(const String& digits, const StringListMatrix& m, StringList& os) const {
        combineWords(0, digits, m, String(), os);
    }
    void combineWords(int startpos, const String& digits, const StringListMatrix& m, String pre, StringList& os) const {
        int NR = m.NROW;
        int NC = m.NCOL;
        static const char SEP='-';
        if( startpos == digits.length() ) { // end of string, print
            Stringstream ss;
            // remove unnecessary SEP
            int i = 0;
            while(pre[i++] == SEP) ;
            --i;
            for(; i< pre.length(); ++i) {
                if( pre[i] == SEP && (pre[i+1] == SEP || 0==(isalpha(pre[i-1]) + isalpha(pre[i+1]))) ) {
                    continue;
                }
                ss << pre[i];
            }
            os.push_back(ss.str());
            return;
        }
        // search for next matched word
        int minStep = 0;
        int minStart = startpos;
        for(int j=startpos; j<NC && minStep==0; ++j) {
            for(int i=minWordLen; i<NR; ++i) {
                if( m(i,j).size() > 0 ) {
                    String w = pre + SEP + digits.substr(startpos, j-startpos) + SEP;
                    for(StringList::const_iterator it=m(i,j).begin(); it != m(i,j).end(); ++it) {
                        combineWords(j+i, digits, m, w + (*it), os);
                    }
                    if( minStep == 0 ) {
                        minStep = i;
                        minStart = j;
                    }
                }
            }
        }
        if( minStep == 0 ) {
            combineWords(digits.length(), digits, m, pre + SEP + digits.substr(startpos, digits.length()-startpos), os);
        }else{
            // search for next match with starting position before minStep
            for(int j=minStart+1; j<=minStep; ++j) {
                for(int i=minWordLen; i<NR; ++i) {
                    if( m(i,j).size() > 0 ) {
                        combineWords(j, digits, m, pre + SEP + digits.substr(startpos, j-startpos), os);
                        return;
                    }
                }
            }
        }
    }

    StringList words;
    CharCharMap a2d; // letter 2 digit
    StringStringListMap n2w; // number 2 word
};


void print_usage()
{
    const char* program = "program";
    printf("Usage: %s [OPTIONS] [numbers]\n", program);
    printf("Find words hidden inside phone numbers (separated by comma).\n\n");
    printf("  If nubmers are read via stdin, two consecutive empty lines terminate input.\n");
    printf(" -d <dictionary> File to use as dictionary (Default: /usr/share/dict/words)\n");
    printf(" -w mininum word length (Default: 2)\n");
    printf("\nExample:\n");
    printf(" %s 2255.63,7292650782\n", program);

}

int run(int argc, const char* argv[])
{
    const char *dictname=NULL;
    String number;
    for(int i=1; i<argc; ++i) {
        if( 0 == strcmp(argv[i], "-n") ) {
            ++i;
            dictname = argv[i];
        }else if( 0 == strcmp(argv[i], "-h")
                  || 0 == strcmp(argv[i], "-?")
                  || 0 == strcmp(argv[i], "--help")) {
            print_usage();
            return 0;
        }else{
            number = argv[i];
        }
    }
    if( number.empty() ) {
        String prev="a";
        String s;
        while (getline( std::cin, s ) && (!s.empty() || !prev.empty()) ) { // exit reading on two consecutive empty lines.
            number += s;
            prev = s;
        }
    }

    jz::PhoneNumberWord pnw;
    long long time0, time1, time2;
#ifdef TIME_IT
    time0 = current_timestamp();
#endif
    bool ok = dictname == NULL ? pnw.loadDict(): pnw.loadDict(dictname);
    if( !ok ) {
        printf("Failed to read dict file!\n");
        return -1;
    }
#ifdef TIME_IT
    time1 = current_timestamp();
    printf("dict loading time: %lld\n", time1-time0);
#endif

   const char DEL = ',';
   for(int currPos = 0; currPos<number.length(); ++currPos) {
        int pos = number.find_first_of(DEL, currPos);
        if( pos == String::npos ) {
            pos =  number.length();
        }
        String num(number, currPos, pos-currPos);
        Cout << num << std::endl;
        pnw.findWord(num, Cout);

        currPos = pos;
    }
#ifdef TIME_IT
   time2 = current_timestamp();
   printf("process loading time: %lld\n", time2-time1);
#endif
    return 0;
}

void test() {
    PhoneNumberWord pnw;
    StringList words;
    pnw.loadDict();
    pnw.findWord(_T("7292650782"), Cout);
    pnw.findWord(_T("2255.63"), Cout);
}
} // namespace jz

int main(int argc, const char* argv[])
{
    return jz::run(argc, argv);
}
