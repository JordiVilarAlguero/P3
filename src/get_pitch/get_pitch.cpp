#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <vector>
#include <algorithm>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"
#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    --llindar-rmax FLOAT  llindar de decisio sonor, sord per a rmax [default: 0.5]
    --llindar-pot FLOAT  llindar de decisio sonor, sord per a pot [default: -45]
    --llindar-r1norm FLOAT  llindar de decisio sonor, sord per a r1norm [default: 0.75]
    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

// Filtrado paso bajo (media móvil)
void low_pass_filter(vector<float>& signal, int window_size) {
    vector<float> filtered_signal(signal.size());
    
    for (size_t i = 0; i < signal.size(); ++i) {
        float sum = 0.0f;
        int count = 0;
        
        // Promediamos los valores en la ventana
        for (int j = -window_size / 2; j <= window_size / 2; ++j) {
            int index = i + j;
            if (index >= 0 && index < signal.size()) {
                sum += signal[index];
                count++;
            }
        }
        filtered_signal[i] = sum / count;
    }
    
    // Asignamos el resultado al vector original
    signal = filtered_signal;
}

// Filtro de mediana
void median_filter(vector<float>& f0, int filter_size) {
    vector<float> f0_filtered(f0.size());

    for (size_t i = 0; i < f0.size(); ++i) {
        vector<float> window;

        // Crear ventana de tamaño `filter_size`
        for (int j = -filter_size / 2; j <= filter_size / 2; ++j) {
            if (i + j >= 0 && i + j < f0.size()) {
                window.push_back(f0[i + j]);
            }
        }

        // Ordenamos los elementos de la ventana
        sort(window.begin(), window.end());

        // Asignamos el valor mediano
        f0_filtered[i] = window[window.size() / 2];
    }

    // Asignamos el resultado al vector original
    f0 = f0_filtered;
}

int main(int argc, const char *argv[]) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},    // array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

    std::string input_wav = args["<input-wav>"].asString();
    std::string output_txt = args["<output-txt>"].asString();
    float llindar_rmax = stof(args["--llindar-rmax"].asString());

    // Read input sound file
    unsigned int rate;
    vector<float> x;
    if (readwav_mono(input_wav, rate, x) != 0) {
        cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
        return -2;
    }

    int n_len = rate * FRAME_LEN;
    int n_shift = rate * FRAME_SHIFT;

    // Define analyzer
    PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::RECT, 50, 500, llindar_rmax);

    // Preprocess the input signal: low-pass filtering
    low_pass_filter(x, 5);  // Apply a simple low-pass filter with a window size of 5

    // Iterate for each frame and save values in f0 vector
    vector<float>::iterator iX;
    vector<float> f0;
    for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
        float f = analyzer(iX, iX + n_len);
        f0.push_back(f);
    }

    // Postprocess the estimation: apply median filter
    median_filter(f0, 3);  // Apply median filter with a window size of 3

    // Write f0 contour into the output file
    ofstream os(output_txt);
    if (!os.good()) {
        cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
        return -3;
    }

    os << 0 << '\n';  // pitch at t=0
    for (iX = f0.begin(); iX != f0.end(); ++iX)
        os << *iX << '\n';
    os << 0 << '\n';  // pitch at t=Dur

    return 0;
}
