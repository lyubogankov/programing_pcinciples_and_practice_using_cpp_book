/*
5. Write the function print_year() mentioned in §10.11.2.
*/

/*
Input / output format:
{ year <int> { month <3-letter str> {<int day> <int hour/24> <double temp-reading>}* }*}
            ^                      ^ could be whitespace / newlines here
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// in-memory representation ---------------------------------------------------
const int not_a_reading = -7777;
const int not_a_month = -1;
struct Day {
    // initialize with 24x "not a reading"
    vector<double> hour {vector<double>(24, not_a_reading)};
    bool has_valid_readings() {
        for (double& r : hour)
            if (r != not_a_reading) return true;
        return false;
    }
};
struct Month {
    int month {not_a_month}; // [0, 11] where jan=0
    vector<Day> day {32}; // [1, 31] (not using idx 0)
    bool has_valid_readings() {
        for (int i=1; i<32; i++)
            if (day[i].has_valid_readings()) return true;
        return false;
    }
};
struct Year {
    int year;
    vector<Month> month {12};
    bool has_valid_readings() {
        for (Month& m : month)
            if (m.month != not_a_month) return true;
        return false;
    }
};
bool operator<(Year& y1, Year& y2) { return y1.year < y2.year; }
bool operator>(Year& y1, Year& y2) { return y1.year > y2.year; }

const vector<string> month_input_tbl = {
        "jan", "feb", "mar", "apr", "may", "jun",
        "jul", "aug", "sep", "oct", "nov", "dec"
};
int month_to_int(string month) {
    for (int i=0; i<12; ++i) if (month_input_tbl[i] == month) return i;
    return not_a_month;
}
string int_to_month(int month) {
    if (month < 0 || month > 12) throw runtime_error("bad month index");
    return month_input_tbl[month];
}

// input from file ------------------------------------------------------------
struct Reading {
    int day;
    int hour;
    double temperature;
};
constexpr int implausible_temperature_min = -200;
constexpr int implausible_temperature_max =  200;
bool is_valid(const Reading& r) {
    if (r.day < 1 || 31 < r.day) return false;
    if (r. hour < 0 || 24 < r.hour) return false;
    if (r.temperature < implausible_temperature_min 
        || implausible_temperature_max < r.temperature) return false;
    return true;
}
istream& operator>>(istream& is, Reading& r)
// checking format, but not bothering with validity
{
    char ch1;
    if (is >> ch1 && ch1 != '(') {
        is.unget();
        is.clear(ios_base::failbit);
        return is;
    }
    char ch2;
    int d, h;
    double t;
    is >> d >> h >> t >> ch2;
    if (!is || ch2 != ')') throw runtime_error("bad reading");
    r.day = d;
    r.hour = h;
    r.temperature = t;
    return is;
}

void end_of_loop(istream& ist, char term, const string& message)
// Check for terminator when input stream is in failed state
{
    if (ist.fail()) {
        ist.clear();
        char ch;
        if (ist >> ch && ch == term) return;
        throw runtime_error(message);
    }
}
istream& operator>>(istream&is, Month& m) {
    char ch = 0;
    if (is >> ch && ch != '{') {
        is.unget();
        is.clear(ios_base::failbit);
        return is;
    }
    // month
    string month_marker, mm;
    is >> month_marker >> mm;
    if (!is || month_marker != "month") throw runtime_error("bad reading");
    m.month = month_to_int(mm);
    // arbitrary # of readings (0+)
    int duplicates=0, invalids=0;
    for(Reading r; is >> r;) {
        if (is_valid(r)) {
            if (m.day[r.day].hour[r.hour] != not_a_reading)
                ++duplicates;
            m.day[r.day].hour[r.hour] = r.temperature;
        } else
            ++invalids;
    }
    if (invalids) throw runtime_error("invalid readings in month");
    if (duplicates) throw runtime_error("duplicate readings in month");
    end_of_loop(is, '}', "bad end of month");
    return is;
}
istream& operator>>(istream&is, Year& y) {
    char ch;
    if (is >> ch && ch != '{') {
        is.unget();
        is.clear(ios_base::failbit);
        return is;
    }
    string year_marker;
    int yy;
    is >> year_marker >> yy;
    if (!is || year_marker != "year") throw runtime_error("bad reading");
    y.year = yy;
    // read 0-12 months
    // the book's code doesn't check for duplicate months
    while (true) {
        Month m;
        if(!(is >> m)) break;
        y.month[m.month] = m;
    }
    end_of_loop(is, '}', "bad end of year");
    return is;
}

// output from file -----------------------------------------------------------
const string INDENT_UNIT = "    ";
void print_month(ofstream &ofs, Month& m, string indent = INDENT_UNIT) {
    // month: start
    ofs << "\n" + indent + "{ month " << int_to_month(m.month) << " ";
    // if at least one day present
    // month: end
    ofs << '}';
}

void print_year(ofstream& ofs, Year& y)
// The textbook doesn't clearly define what this function should do, but 
// based on context clues (we're passing in an ofstream -> file) it seems
// like we should write the in-memory representation to a file.
{
    // year: start
    ofs << "{ year " << y.year << " ";
    // if least one month present
    if (y.has_valid_readings()) {
        for (Month& m : y.month)
            if (m.month != not_a_month)
                print_month(ofs, m);
    }
    // year: end
    ofs << '}';
    return
}


int main() {

    // open an input file
    cout << "Please enter input file name:  ";
    string iname;
    cin >> iname;
    ifstream ifs {iname};
    if (!ifs) throw runtime_error("Can't open file for reading (" + iname + ")");
    // will throw error if in bad() stream state
    ifs.exceptions(ifs.exceptions() | ios_base::badbit);

    // open an output file
    cout << "Please enter output file name: ";
    string oname;
    cin >> oname;
    ofstream ofs {oname};
    if (!ofs) throw runtime_error("Can't open file for writing (" + oname + ")");

    // read from input
    vector<Year> years;
    while (true) {
        Year y;
        if(!(ifs >> y)) break;
        years.push_back(y);
    }
    cout << "... read " << years.size() << " years of readings from input file.\n";

    // ** my addition -- sorting the vector of years.
    // months/days/hours are already sorted (within in-memory representation)
    // and I want years to be as well!
    sort(years.begin(), years.end());

    // write to output
    for (Year& y : years)
        print_year(ofs, y);
    

    return 0;
}