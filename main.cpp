#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <complex>
#include <valarray>
#include <cmath>

 
using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::string;



// Function prototypes
int getFileSize(FILE* inFile);
int fileDetail(char* );
// std::vector<std::complex<double>> performFFT(const std::vector<int8_t>& data)


namespace little_endian_io
{
 template <typename Word>
 std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
 {
   for (; size; --size, value >>= 8)
     outs.put( static_cast <char> (value & 0xFF) );
   return outs;
 }
}


using namespace std;
using little_endian_io::write_word;



// // find the file size
int getFileSize(FILE* inFile)
{
   int fileSize = 0;
   fseek(inFile, 0, SEEK_END);
 
   fileSize = ftell(inFile);
 
   fseek(inFile, 0, SEEK_SET);
   return fileSize;
}


typedef struct  WAV_HEADER
{
   /* RIFF Chunk Descriptor */
   uint8_t         RIFF[4];        // RIFF Header Magic header
   uint32_t        ChunkSize;     // RIFF Chunk Size
   uint8_t         WAVE[4];        // WAVE Header
   /* "fmt" sub-chunk */
   uint8_t         fmt[4];         // FMT header
   uint32_t        Subchunk1Size;  // Size of the fmt chunk
   uint16_t        AudioFormat;   // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
   uint16_t        NumOfChan;     // Number of channels 1=Mono 2=Sterio
   uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
   uint32_t        bytesPerSec;   // bytes per second
   uint16_t        blockAlign;    // 2=16-bit mono, 4=16-bit stereo
   uint16_t        bitsPerSample;  // Number of bits per sample
   /* "data" sub-chunk */
   uint8_t         Subchunk2ID[4]; // "data"  string
   uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;
 

 
int main()
{
   wav_hdr wavHeader;
   int headerSize = sizeof(wav_hdr), filelength = 0;
 
   const char* filePath = "./1.wav";

 
   FILE* wavFile = fopen(filePath, "r");
   if (wavFile == nullptr)
   {
       fprintf(stderr, "Unable to open wave file: %s\n", filePath);
       return 1;
   }
 
   //Read the header
   size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
//    cout << "Header Read " << bytesRead << " bytes." << endl;

   // writing the file
    std::ofstream f( "example0.wav", ios::binary );
 
    // Write the file headers
    f << "RIFF----WAVEfmt ";    // (chunk size to be filled in later)
    write_word( f,     16, 4 );  // no extension data
    write_word( f,      1, 2 );  // PCM - integer samples
    write_word( f,      2, 2 );  // two channels (stereo file)
    write_word( f,  44100, 4 );  // samples per second (Hz)
    // write_word( f, 176400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f, 2822400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word( f,     16, 2 );  // number of bits per sample (use a multiple of 8)
    
    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)


   if (bytesRead > 0)
   {
       //Read the data
       uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;     //Number     of bytes per sample
       uint64_t numSamples = wavHeader.ChunkSize / bytesPerSample; //How many samples are in the wav file?
       static const uint16_t BUFFER_SIZE = 4096;
       int8_t* buffer = new int8_t[BUFFER_SIZE];
       std::vector<int8_t> totalBuffer(fileDetail("./1.wav"));
       int r = 0;
       std::vector<int8_t>::iterator it = totalBuffer.begin();
       while ((bytesRead = fread(buffer, sizeof buffer[0], BUFFER_SIZE / (sizeof buffer[0]), wavFile)) > 0)
       {    
           std::copy(buffer, buffer+BUFFER_SIZE, it+(BUFFER_SIZE*r));
           r++;
       }

    
    // performFFT(totalBuffer);


    //  writing the .wav data
    while (it != totalBuffer.end()) {
        int8_t temp = *it;
        // cout << temp;
        f.put( static_cast <int8_t> (temp));
        it++;
    }




       delete [] buffer;
       buffer = nullptr;
       filelength = getFileSize(wavFile);
       cout<<filelength;
   }
   fclose(wavFile);
   return 0;
}




 

int fileDetail(char* filePath)
{
   wav_hdr wavHeader;
   int headerSize = sizeof(wav_hdr), filelength = 0;
 
   string input;
 
   FILE* wavFile = fopen(filePath, "r");
   if (wavFile == nullptr)
   {
       fprintf(stderr, "Unable to open wave file: %s\n", filePath);
       return 1;
   }
 
   //Read the header
   size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
   if (bytesRead > 0)
   {
       //Read the data
       uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;     //Number     of bytes per sample
       uint64_t numSamples = wavHeader.ChunkSize / bytesPerSample; //How many samples are in the wav file?
       static const uint16_t BUFFER_SIZE = 4096;
       int8_t* buffer = new int8_t[BUFFER_SIZE];
       while ((bytesRead = fread(buffer, sizeof buffer[0], BUFFER_SIZE / (sizeof buffer[0]), wavFile)) > 0);
       delete [] buffer;
       buffer = nullptr;
       filelength = getFileSize(wavFile);
   }
   fclose(wavFile);
   cout<<"\n\nwavHeader.Subchunk2Size: "<<wavHeader.Subchunk2Size<<std::endl;
   return wavHeader.Subchunk2Size;
}
 





// Function to perform FFT
void fft(std::valarray<std::complex<double>>& x) {
    const size_t N = x.size();
    if (N <= 1) return;

    // Divide
    std::valarray<std::complex<double>> even = x[std::slice(0, N / 2, 2)];
    std::valarray<std::complex<double>> odd = x[std::slice(1, N / 2, 2)];

    // Conquer
    fft(even);
    fft(odd);

    // Combine
    for (size_t k = 0; k < N / 2; ++k) {
        std::complex<double> t = std::polar(1.0, -2.0 * M_PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}

// Function to perform Fourier transform on int8_t data
std::vector<std::complex<double>> performFFT(const std::vector<int8_t>& data) {
    // Convert int8_t data to std::complex<double>
    std::valarray<std::complex<double>> complexData(data.begin(), data.end());

    // Perform FFT
    fft(complexData);

    // Convert result to vector
    std::vector<std::complex<double>> result(std::begin(complexData), std::end(complexData));
    return result;
}