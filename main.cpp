#include "histogram.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include <sstream>
#include <string>

using namespace std;

struct Input
{
    vector<double> numbers;
    size_t bin_count;
};

vector<double>input_numbers(istream& in, size_t count)
{
    vector<double> result(count);
    for (size_t i = 0; i < count; i++)
    {
        in >> result[i];
    }
    return result;
}

Input read_input(istream& in, bool prompt)
{
    Input data;
    if (prompt)
        cerr << "Enter number count: ";
    size_t number_count;
    in >> number_count;
    if (prompt)
        cerr << "Enter numbers: ";
    data.numbers = input_numbers(in, number_count);
    if (prompt)
        cerr << "enter bin count: ";
    in >> data.bin_count;
    return data;
}

vector<size_t>make_histogram(Input data)
{
    double min = data.numbers[0];
    double max = data.numbers[0];
    find_minmax(data.numbers, min, max);
    vector<size_t> bins(data.bin_count);
    for (double number : data.numbers)
    {
        size_t bin = (size_t)((number - min) / (max - min) * data.bin_count);
        if (bin == data.bin_count)
        {
            bin--;
        }
        bins[bin]++;
    }
    return bins;
}

void show_histogram_text (const vector<size_t>& bins)
{
    const size_t SCREEN_WIDTH = 80;
    const size_t MAX_ASTERISK = SCREEN_WIDTH - 4 - 1;

    size_t max_count = 0;
    for (size_t count : bins)
    {
        if (count > max_count)
        {
            max_count = count;
        }
    }
    const bool scaling_needed = max_count > MAX_ASTERISK;

    for (size_t bin : bins)
    {
        if (bin < 100)
        {
            cout << ' ';
        }
        if (bin < 10)
        {
            cout << ' ';
        }
        cout << bin << "|";

        size_t height = bin;
        if (scaling_needed)
        {
            const double scaling_factor = (double)MAX_ASTERISK / max_count;
            height = (size_t)(bin * scaling_factor);
        }

        for (size_t i = 0; i < height; i++)
        {
            cout << '*';
        }
        cout << '\n';
    }
}

size_t write_data(void* items, size_t item_size, size_t item_count, void* ctx)
{
    stringstream* buffer = reinterpret_cast<stringstream*>(ctx);
    size_t data_size = item_size * item_count;
    buffer->write((char*)items, data_size);
    return data_size;
}

Input download(const string& address)
{
    stringstream buffer;
    CURL *curl = curl_easy_init();
    if(curl)
    {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            cerr << curl_easy_strerror(res) << endl;
            exit(1);
        }
        curl_easy_cleanup(curl);
    }
    return read_input(buffer, false);
}

int main(int argc, char* argv[])
{
    Input input;
    bool format_is_svg = true;
    string url;
    if (argc > 1)
    {
        if (string(argv[argc - 1]) == "-format")
        {
            cerr << "input histogramm format ('text' or 'svg') after [-format] argument" << endl;
            exit(1);
        }
        for (int i = 0; i < argc; i++)
        {
            if (string(argv[i]) == "-format")
            {
                if (string(argv[i + 1]) == "text")
                {
                    format_is_svg = false;
                }
                else if (string(argv[i + 1]) == "svg" )
                {
                    format_is_svg = true;
                }
                else
                {
                    cerr << "input histogramm format ('text' or 'svg') after [-format] argument" << endl;
                    exit(1);
                }
            }
            else if (strstr(argv[i], "http") != NULL)
            {
                url = argv[i];
            }
        }
        input = download(url);
    }
    else
    {
        input = read_input(cin, true);
    }
    // Расчет гистограммы
    const auto bins = make_histogram(input);
    //Вывод гистограммы
    if (format_is_svg)
        show_histogram_svg(bins);
    else
        show_histogram_text(bins);
    return 0;
}
