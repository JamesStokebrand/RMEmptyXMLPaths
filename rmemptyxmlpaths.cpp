/*
##
##   Copyright Nov 2012 James Stokebkrand
##
##   Licensed under the Apache License, Version 2.0 (the "License");
##   you may not use this file except in compliance with the License.
##   You may obtain a copy of the License at
##
##       http://www.apache.org/licenses/LICENSE-2.0
##
##   Unless required by applicable law or agreed to in writing, software
##   distributed under the License is distributed on an "AS IS" BASIS,
##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##   See the License for the specific language governing permissions and
##   limitations under the License.
##
##
## Filename: rmemptypaths.cpp
##  
## Purpose:  The purpose of this C++ program is to read, parse and remove the empty 
##             XML paths from an SVG file created by the INKSCAPE vector graphics 
##             program.  
##
## Questions?  Comments?  Complaints?  Please feel free to contact me:
##   jamesstokekbrand (AT) g_m_a_i_l (DOT) com
## 
## Replace the (AT) and (DOT) with the appropriate symbols.  Remove the "_" and enjoy.
##
*/


#include <string>
#include <iostream>
#include <fstream>

using namespace std;

// fileIO class handles file reads/writes
class fileIO
{
public:

    enum fileIOStatus {
         SUCCESS=0
        ,FAILURE=1
        ,FILE_EOF=2
    };

    fileIO(string anInFile, string anOutFile)
    : inFileName(anInFile)
    , outFileName(anOutFile)
    { }

    ~fileIO()
    {
        if (inFile.is_open()) inFile.close();
        if (outFile.is_open()) outFile.close();
    };

    fileIOStatus init()
    {
        inFile.open(inFileName.c_str() , ios::in);
        outFile.open(outFileName.c_str() , ios::out|ios::trunc); 
        if (inFile.is_open() && outFile.is_open()) return SUCCESS;
        return FAILURE;
    };

    fileIOStatus read(unsigned char &aChar)
    {
        if (inFile.is_open())
        {
            aChar = inFile.get();
            if (inFile.eof()) return FILE_EOF;
            return SUCCESS;
        }
        return FAILURE;
    };

    fileIOStatus write(unsigned char &aChar)
    {
        if (outFile.is_open())
        {
            outFile << aChar;
            return SUCCESS;
        }
        return FAILURE;
    }

    fileIOStatus write(string &aString)
    {
       if (outFile.is_open())
       {
           outFile << aString;
           return SUCCESS;
       }
       return FAILURE;
    }

private:
    string inFileName;
    string outFileName;

    ifstream inFile;
    ofstream outFile;
};

// XML_Parser searches the input file for the empty paths, 
// pipes the output minus the empty paths to the out file.
class XML_Parser
{
public:
    bool init(string inputFileName, string outputFileName)
    {
        myFileIO = new fileIO(inputFileName,outputFileName);
        if ((myFileIO) && (myFileIO->init() == fileIO::SUCCESS)) return true;

        // If we got here, this means the fileIO class failed to init or failed the new.
        if (myFileIO) delete myFileIO;
        return false;
    };

    typedef bool (XML_Parser::*State)(unsigned char aChar);

    bool seeking(unsigned char aChar)
    {
        switch(aChar)
        {
        case '<':
            // '<' symbol marks the beginning of an XML tag.
            // Change to char_p state to seek the 'p' char that marks
            //  the beginning of the '<path" xml tag.
            change_state(&XML_Parser::char_p, "char_p", aChar);
            return true;
            break;
        default:
            // This is not the desired char, flush this to the output file.
            return flush();
            break;
        };
        return false;
    };

    bool char_p(unsigned char aChar)
    {
        switch(aChar)
        {
        case 'p':
            change_state(&XML_Parser::char_a, "char_a", aChar);
            return true;
            break;
        default:
            // This is not the desired char, change state back to seeking
            // and flush the accumuted chars to the output file.
            change_state(&XML_Parser::seeking, "seeking", aChar);
            return flush();
            break;
        };
        return false;
    }

    bool char_a(unsigned char aChar)
    {
        switch(aChar)
        {
        case 'a':
            change_state(&XML_Parser::char_t, "char_t", aChar);
            return true;
            break;
        default:
            // This is not the desired char, change state back to seeking
            // and flush the accumuted chars to the output file.
            change_state(&XML_Parser::seeking, "seeking", aChar);
            return flush();
            break;
        };
        return false;
    }

    bool char_t(unsigned char aChar)
    {
        switch(aChar)
        {
        case 't':
            change_state(&XML_Parser::char_h, "char_h", aChar);
            return true;
            break;
        default:
            // This is not the desired char, change state back to seeking
            // and flush the accumuted chars to the output file.
            change_state(&XML_Parser::seeking, "seeking", aChar);
            return flush();
            break;
        };
        return false;
    }

    bool char_h(unsigned char aChar)
    {
        switch(aChar)
        {
        case 'h':
            change_state(&XML_Parser::char_first_zero, "char_first_zero", aChar);
            return true;
            break;
        default:
            // This is not the desired char, change state back to seeking
            // and flush the accumuted chars to the output file.
            change_state(&XML_Parser::seeking, "seeking", aChar);
            return flush();
            break;
        };
        return false;
    }

    bool char_first_zero(unsigned char aChar)
    {
        // Found the start of the '<path' XML tag.  Seek either the '0,0"' strings or
        //  the '/>' which marks the end of the XML tag.
        switch(aChar)
        {
        case '/':
            // This may be the end of the XML path, seek the '>' symbol
            change_state(&XML_Parser::char_greater_than, "char_greater_than", aChar);
            return true;
            break;
        case '0':
            // Found the possible start of the '0,0"' empty path, change state
            // to seek the comma.
            change_state(&XML_Parser::char_comma, "char_comma", aChar);
            return true;
            break;
        default:
            // Comma or Slash not found .. keep accumulating the XML string
            return true;
            break;
        };
        return false;
    };

    bool char_comma(unsigned char aChar)
    {
        switch(aChar)
        {
        case '/':
            // This may be the end of the XML path, seek the '>' symbol
            change_state(&XML_Parser::char_greater_than, "char_greater_than", aChar);
            return true;
            break;
        case ',':
            // Found the "," symbol, seek the second zero
            // to seek the comma.
            change_state(&XML_Parser::char_second_zero, "char_second_zero", aChar);
            return true;
            break;
        default:
            // Didn't find a ',', return to seeking the first zero.
            change_state(&XML_Parser::char_first_zero, "char_first_zero", aChar);
            return true;
            break;
        };
        return false;
    };

    bool char_greater_than(unsigned char aChar)
    {
        switch(aChar)
        {
        case '>':
            // This marks the end of an XML tag.  If we are here, we didn't find
            // an emtpy XML path, change state to seeking and flush the accumulated
            // chars.
            change_state(&XML_Parser::seeking, "seeking", aChar);
            return flush();
            break;
        case '0':
            // Didnt find the '>' symbol, but did find a zero, change states to seeking the ',' symbol.
            change_state(&XML_Parser::char_comma, "char_comma", aChar);
            return true;
            break;
        default:
           // Didn't find a '>' symbol, this is not the end of the XML path, return to seeking the first zero.
           change_state(&XML_Parser::char_first_zero, "char_first_zero", aChar);
           return true;
           break;
        };
        return false;
    };


    bool char_second_zero(unsigned char aChar)
    {
        switch(aChar)
        {
        case '/':
            // This may be the end of the XML path, seek the '>' symbol
            change_state(&XML_Parser::char_greater_than, "char_greater_than", aChar);
            return true;
            break;
        case '0':
            // Found the second zero, seek the end quote that would identify it as an empty path
            change_state(&XML_Parser::char_quote, "char_quote", aChar);
            return true;
            break;
        default:
           // Didn't find a '>' symbol, this is not the end of the XML path, return to seeking the first zero.
           change_state(&XML_Parser::char_first_zero, "char_first_zero", aChar);
           return true;
           break;
        };
    };

    bool char_quote(unsigned char aChar)
    {
        switch(aChar)
        {
        case '/':
            // This may be the end of the XML path, seek the '>' symbol
            change_state(&XML_Parser::char_greater_than, "char_greater_than", aChar);
            return true;
            break;
        case '"':
            // Found the quote, this is an empty path.  Change state to seek the end of path marker
            change_state(&XML_Parser::char_slash_after_zero, "char_slash_after_zero", aChar);
            return true;
            break;
        default:
           // Didn't find a '>' symbol, this is not the end of the XML path, return to seeking the first zero.
           change_state(&XML_Parser::char_first_zero, "char_first_zero", aChar);
           return true;
           break;
        };
        return false;
    }

    bool char_slash_after_zero(unsigned char aChar)
    {
        // If we are here, we have found an XML path that is emtpy, accumulate all chars till the end of 
        // of the XML string.
        switch(aChar)
        {
        case '/':
            // Is this the end of the XML tag?  Change states to seek the '>' symbol.
            change_state(&XML_Parser::char_greater_than_after_zero, "char_greater_than_after_zero", aChar);
            return true;
            break;
        default:
            // Already know this is an emtpy path, accumulate the rest of the XML tag.
            return true;
            break;
        };
        return false;
    };

    bool char_greater_than_after_zero(unsigned char aChar)
    {
        switch(aChar)
        {
        case '>':
            // Found the end of an empty XML tag.  Discard this XML tag.
#if 0
            cout << "Empty XML tag found: " << endl ;
            cout << "Empty XML tag #########################" << endl;
            cout << current_buffer << endl;
            cout << "#######################################" << endl << endl;
#endif
            empty_paths_found++;
            current_buffer = "";  // Discard the accumulated XML tag.  Go back to seeking.
            change_state(&XML_Parser::seeking, "seeking", aChar);
            return true;
            break;
        default:
            // Didnt find the wanted char, return to seeking the end of the XML tag.
            change_state(&XML_Parser::char_slash_after_zero, "char_slash_after_zero", aChar);
            return true;
            break;
        }; 
        return false;
    }

    bool process()
    {
        unsigned char aChar;
        if (myFileIO->read(aChar) == fileIO::SUCCESS)
        {
            current_buffer += aChar;
            //cout << "Process char:" << aChar << endl;
            //cout << "Current string:" << current_buffer << endl;
            return (this->*current_state)(aChar);
        }
        return false;
        
    }

    void change_state(State new_state, string theState="", unsigned char aChar = '0')
    {
        if (theState.length() > 0) 
        {
            //cout << "change_state :" << theState << " char:" << aChar << endl;
        };
        current_state = new_state;
    }

    void status()
    {
        cout << "Found " << empty_paths_found << " empty paths." << endl;
    }

    XML_Parser()
    : myFileIO(NULL)
    , current_state(&XML_Parser::seeking)
    , current_buffer("")
    , empty_paths_found(0)
    {}

    ~XML_Parser()
    {
       if(myFileIO) delete myFileIO;
    }

    bool flush()
    {
        if ((myFileIO) && (myFileIO->write(current_buffer)==fileIO::SUCCESS))
        {
            current_buffer = ""; // Reset the current buffer
            return true;
        }
        return false;
    }

private:
    fileIO *myFileIO;
    State current_state;
    string current_buffer;
    int empty_paths_found;
};

int main(int argc, char *argv[])
{

/*
    // Debug code
    cout << "Argc:" << argc << endl;
    for (int ii=0; ii<argc; ii++)
    {
        cout << "argv[" << ii << "] " << argv[ii] << endl;
    }
*/

    if (argc <= 1)
    {
        cout << "No filename provided.\nUsage " << argv[0] << " <input_filename.svg>\n" << endl;
        cout << "Given an input SVG file, it will scan it for emtpy XML paths and remove them.  Placing the output" << endl;
        cout << "  in <input_filename.svg>_out.svg" << endl;
 
        return 1;
    }

    string inFileName = argv[1];
    string outFileName = inFileName + "_out.svg";

    cout << " inFileName :" << inFileName << endl;
    cout << "outFileName :" << outFileName << endl;

    XML_Parser an_XML_Parser;

    // Set the input and output filenames
    an_XML_Parser.init(inFileName,outFileName);

    // Process the file char by char
    while (an_XML_Parser.process()) {};

    an_XML_Parser.status();

    return 0;
}


