#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif
using namespace std;

class AppView {
public:
	AppView() {}

	void print(string msg) {
		cout << msg;
	}

	void printLine(string msg) {
		cout << msg << endl;
	}

	void print_division() {
		printLine("----------------------------------------------------------------------------");
	}

	void print_guide() {
		print_division();
		printLine(u8"n:다음페이지, p:이전페이지, i:삽입, d:삭제, c:변경, s:찾기, t:저장 후 종료");
		print_division();
	}

	void print_message(string msg) {
		print(u8"(콘솔메시지) ");
		printLine(msg);
		print_division();
	}

	void print_page(vector<string> rows, int page) {
		if (page == rows.size() / 20 + 1) {
			for (size_t i = rows.size() - 20; i < rows.size(); i++) {
				printLine(rows[i]);
			}
		}
		else {
			for (int i = page * 20 - 20; i < page * 20; i++) {
				printLine(rows[i]);
			}
		}
	}

	string inputLine() {
		string input;
		getline(cin, input);
		while (input == "") {
			getline(cin, input);
		}
		return input;
	}
}; // end of View

enum class EditorError {
	Undefined,
	UndefinedInsError,
	ArgTypeError,
	ArgNumberError,
	ArgRangeError,
	InputFormatError,
	NothingToChangeError,
	CannotFindError,
	StringTooLongError
};

class EditorException : public exception {
	EditorError error;
public:
	EditorException() : error(EditorError::Undefined) {}
	EditorException(EditorError givenError) : error(givenError) {}
	EditorError getError() {
		return error;
	}
};

class Editor {
	string file_name;
	string line;
	vector<string> rows;

	void replaceAll(string& str, string from, string to) {
		if (from.length() > from.max_size() || to.length() > to.max_size()) {
			throw EditorException(EditorError::StringTooLongError);
		}

		size_t pos;
		size_t offset = 0;
		int count = 0;
		while ((pos = str.find(from, offset)) != string::npos) {
			str.replace(str.begin() + pos, str.begin() + pos + from.size(), to);
			offset = pos + to.size();
			count++;
		}

		if (count == 0) {
			throw EditorException(EditorError::NothingToChangeError);
		}
	}

	void readFile() {
		ifstream ifs(file_name);
		if (ifs.is_open()) {
			while (getline(ifs, line)) {
				for (size_t i = 0; i < line.length(); i += 75) {
					rows.push_back(line.substr(i, 75));
				}
			}
		}

		ifs.close();
	}
public:
	Editor(string file_name) : file_name(file_name) {
		readFile();
	}

	vector<string> get_rows() { return rows; } // 75 바이트씩 들어있는 벡터 반환

	vector<string> insert_words(int row, int idx, string words, int page) {
		if (row < 0 || row > 19 || idx < 0 || idx > 75) {
			throw EditorException(EditorError::ArgRangeError);
		}

		if (words.length() > words.max_size()) {
			throw EditorException(EditorError::StringTooLongError);
		}

		if (page == rows.size() / 20 + 1) {
			if (rows[row + (rows.size() - 20)].length() < idx) {
				throw EditorException(EditorError::ArgRangeError);
			}
			string changed = rows[row + (rows.size() - 20)].insert(idx, words);
			rows[row + (rows.size() - 20)] = changed;
		}
		else {
			if (rows[row + (page * 20 - 20)].length() < idx) {
				throw EditorException(EditorError::ArgRangeError);
			}
			string changed = rows[row + (page * 20 - 20)].insert(idx, words);
			rows[row + (page * 20 - 20)] = changed;
		}
		
		line = "";
		for (vector<string>::iterator itr = rows.begin(); itr != rows.end(); ++itr) {
			line += *itr;
		}
		rows.clear();
		for (size_t i = 0; i < line.length(); i += 75) {
			rows.push_back(line.substr(i, 75));
		}

		return rows;
	}

	vector<string> delete_words(int row, int idx, int len, int page) {
		if (row < 0 || row > 19 || idx < 0 || idx > 74) {
			throw EditorException(EditorError::ArgRangeError);
		}

		if (page == rows.size() / 20 + 1) {
			if (rows[row + (rows.size() - 20)].length() < idx) {
				throw EditorException(EditorError::ArgRangeError);
			}
			string changed = rows[row + (rows.size() - 20)].replace(idx, len, "");
			rows[row + (rows.size() - 20)] = changed;
		}
		else {
			if (rows[row + (page * 20 - 20)].length() < idx) {
				throw EditorException(EditorError::ArgRangeError);
			}
			string changed = rows[row + (page * 20 - 20)].replace(idx, len, "");
			rows[row + (page * 20 - 20)] = changed;
		}

		line = "";
		for (vector<string>::iterator itr = rows.begin(); itr != rows.end(); ++itr) {
			line += *itr;
		}
		rows.clear();
		for (size_t i = 0; i < line.length(); i += 75) {
			rows.push_back(line.substr(i, 75));
		}

		return rows;
	}

	vector<string> search_words(string str) {
		vector<string> result;
		size_t idx;
		size_t locatedPage;
		if ((idx = line.find(str)) != string::npos) {
			size_t NumberOfLines = ((idx + str.length()) % 75 == 0) ? (idx + str.length()) / 75 : (idx + str.length()) / 75 + 1;
			locatedPage = (NumberOfLines % 20 == 0) ? (NumberOfLines / 20) : (NumberOfLines / 20 + 1);
			int count = 0;
			for (idx; count < 20; idx += 75) {
				result.push_back(line.substr(idx, 75));
				count++;

				if ((idx + 75) > line.length() - 1)
					break;
			}
		}
		else {
			throw EditorException(EditorError::CannotFindError);
		}

		result.push_back(std::to_string(locatedPage));
		return result;
	}

	vector<string> change_words(string from, string to) {
		replaceAll(line, from, to);
		rows.clear();
		for (size_t i = 0; i < line.length(); i += 75) {
			rows.push_back(line.substr(i, 75));
		}

		return rows;
	}

	void saveFile() {
		ofstream ofs(file_name, ios::out | ios::trunc);
		if (ofs.is_open()) {
			ofs << line;
		}
		ofs.close();
	}
}; // end of Editor

class AppController {
	const string FILE_PATH = "test.txt";
	const string SAVE_AND_QUIT = "t";

	Editor* editor;
	AppView* view;
	vector<string> rows;
	int page = 1;

	void modify_page_num(int row, int idx, string words, int& page) {
		int spells = 0;
		for (int i = 1; i <= row + 1; i++) {
			if (i == row + 1) {
				spells += idx;
			}
			else {
				spells += 75;
			}
		}

		int addedSpells = words.length();
		int totalSpells = spells + addedSpells;

		if (totalSpells >= 1500) {
			if ((totalSpells) % 1500 == 0) {
				page += totalSpells / 1500 - 1;
			}
			else {
				page += totalSpells / 1500;
			}
		}
	}

	void print_previous_page(vector<string> rows, int& page) {
		if (page == 1) {
			view->print_page(rows, page);
			view->print_guide();
			view->print_message("This is the first page!");
		}
		else {
			page--;
			view->print_page(rows, page);
			view->print_guide();
		}
	}

	void print_next_page(vector<string> rows, int& page) {
		if ((rows.size() % 20 == 0 && page == rows.size() / 20) || page == rows.size() / 20 + 1) {
			view->print_page(rows, page);
			view->print_guide();
			view->print_message("This is the last page!");
		}
		else {
			page++;
			view->print_page(rows, page);
			view->print_guide();
		}
	}

	vector<string> split(string str, char delimiter) {
		vector<string> result;
		stringstream ss(str);
		string temp;

		while (getline(ss, temp, delimiter)) {
			result.push_back(temp);
		}

		return result;
	}

	string remove_blank_all(string str) {
		str.erase(remove(str.begin(), str.end(), ' '), str.end());
		return str.c_str();
	}

	string remove_blank_side(string str) {
		size_t lpos = str.find_first_not_of(' ');
		size_t rpos = str.find_last_not_of(' ');

		if (rpos + 1 < str.length()) {
			str.erase(rpos + 1); // 오른쪽 여백 제거
		}

		str.erase(0, lpos); // 왼쪽 여백 제거

		return str.c_str();
	}

	string get_args(string input) { // 실제 인자만 얻어옴
		size_t leftBracketPos = input.find_first_of('(');
		size_t rightBracketPos = input.find_last_of(')');

		if (leftBracketPos == string::npos || rightBracketPos == string::npos) {
			throw EditorException(EditorError::InputFormatError);
		}

		string temp = input.substr(leftBracketPos + 1, rightBracketPos - leftBracketPos - 1);
		return remove_blank_side(temp); // 좌우 여백 제거
	}

	vector<string> parse_args_for_change(string input) { // 변경 기능을 위한 파싱
		vector<string> temp =  split(input, ',');

		if (temp.size() < 2) {
			throw EditorException(EditorError::ArgNumberError);
		}

		temp[0] = remove_blank_side(temp[0]);
		temp[1] = remove_blank_side(temp[1]);
		return temp;
	}

	vector<string> parse_args_for_insert(string input) { // 삽입 기능을 위한 파싱
		vector<string> temp = split(input, ',');

		if (temp.size() < 3) {
			throw EditorException(EditorError::ArgNumberError);
		}

		temp[0] = remove_blank_all(temp[0]);
		temp[1] = remove_blank_all(temp[1]);
		temp[2] = remove_blank_side(temp[2]);
		return temp;
	}

	vector<string> parse_args_for_delete(string input) { // 삭제 기능을 위한 파싱
		string arg = remove_blank_all(input);
		vector<string> temp = split(arg, ',');

		if (temp.size() < 3) {
			throw EditorException(EditorError::ArgNumberError);
		}

		return temp;
	}

	int string_to_integer(string str) { // 문자열 형식의 숫자를 정수형으로 변환
		try {
			int temp = stoi(str);
			return temp;
		}
		catch (...) {
			throw EditorException(EditorError::ArgTypeError);
		}
	}

	void showEditorErrorMessage(EditorError error) { // 각 에러에 따른 메시지 출력
		switch (error) {
		case EditorError::Undefined:
			view->print_message("Undefined Error!");
			break;
		case EditorError::UndefinedInsError:
			view->print_message("Undefined Instruction!");
			break;
		case EditorError::ArgTypeError:
			view->print_message("Type of some argument is not valid!");
			break;
		case EditorError::ArgNumberError:
			view->print_message("Number of Arguments is not enough!");
			break;
		case EditorError::ArgRangeError:
			view->print_message("Some argument is out of range!");
			break;
		case EditorError::InputFormatError:
			view->print_message("Input Format is invalid!");
			break;
		case EditorError::NothingToChangeError:
			view->print_message("There's nothing to change!");
			break;
		case EditorError::CannotFindError:
			view->print_message("Cannot Find the word(s)!");
			break;
		case EditorError::StringTooLongError:
			view->print_message("The length of words to insert is too long!");
			break;
		default:
			break;// nothing to do
		}
	}

	string input_instruction() { // 사용자로부터 입력을 받는 함수
		view->print(u8"입력: ");
		return remove_blank_side(view->inputLine());
	}

public:
	AppController() : editor(new Editor(FILE_PATH)), view(new AppView()) {} // 생성자

	void run() {
		vector<string> args;
		rows = editor->get_rows();
		view->print_page(rows, page);
		view->print_guide();

		string input = input_instruction();
		view->print_division();

		while (input != SAVE_AND_QUIT) {
			try {
				switch (input[0]) {
				case 'p':
					print_previous_page(rows, page);
					break;
				case 'n':
					print_next_page(rows, page);
					break;
				case 'i':
				{
					args = parse_args_for_insert(get_args(input));
					int row = string_to_integer(args[0]) - 1;
					int idx = string_to_integer(args[1]);
					string words = args[2];
					rows = editor->insert_words(row, idx, words, page);

					modify_page_num(row, idx, words, page);

					view->print_page(rows, page);
					view->print_guide();
					break;
				}
				case 'd':
				{
					size_t origin = rows.size();
					args = parse_args_for_delete(get_args(input));
					int row = string_to_integer(args[0]) - 1;
					int idx = string_to_integer(args[1]);
					int len = string_to_integer(args[2]);
					rows = editor->delete_words(row, idx, len, page);

					if (rows.size() % 20 == 0 && rows.size() < origin && page > rows.size() / 20) {
						page--;
					}

					view->print_page(rows, page);
					view->print_guide();

					if (len > 75) 
						view->print_message("Maximum size of deletion is 75 bytes at a time. Only 75 bytes were deleted.");
					break;
				}
				case 'c':
					args = parse_args_for_change(get_args(input));
					rows = editor->change_words(args[0], args[1]);
					view->print_page(rows, page);
					view->print_guide();
					break;
				case 's':
				{
					vector<string> result = editor->search_words(get_args(input));
					size_t size = result.size() - 1;
					page = string_to_integer(result[size]);
					for (int i = 0; i < size; i++) {
						view->printLine(result[i]);
					}
					view->print_guide();
					view->print_message("The word(s) appear on page " + std::to_string(page) + " for the first time.");
					break;
				}
				default:
					throw EditorException(EditorError::UndefinedInsError);
				}
			}
			catch (EditorException& e) {
				view->print_page(rows, page);
				view->print_guide();
				showEditorErrorMessage(e.getError());
			}
			
			input = input_instruction();
			view->print_division();
		}

		editor->saveFile();
	}
}; // end of AppController

int main() {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif

	AppController* controller = new AppController;
	controller->run();

} // end of main