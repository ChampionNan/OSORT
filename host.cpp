#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <random>
#include <boost/program_options.hpp>
// #include <openenclave/host.h>

#include "DataStore.h"
#include "common.h"
#include "enc.h"

// #include "oqsort_u.h"

using namespace std;
using namespace chrono;
namespace po = boost::program_options;

EncOneBlock *arrayAddr[NUM_STRUCTURES];
double IOcost = 0;

po::variables_map read_options(int argc, const char *argv[]) {
  int m, c;
  po::variables_map vm;
  try {
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,H", "Show help message")
    ("memory,M", po::value<int64_t>()->default_value(8), "Internal memory size (MB)")
    ("c,c", po::value<int>()->default_value(16), "The value of N/M")
    ("block_size,B", po::value<int>()->default_value(4), "Block size (in terms of elements)")
    ("num_threads,T", po::value<int>()->default_value(4), "#threads, not suppoted in enclave yet")
    ("sigma,s", po::value<int>()->default_value(40), "Failure probability upper bound: 2^(-sigma)")
    ("alpha,a", po::value<double>()->default_value(-1), "Parameter for ODS")
    ("beta,b", po::value<double>()->default_value(-1), "Parameter for ODS")
    ("gamma,g", po::value<double>()->default_value(-1), "Parameter for ODS")
    ("P,P", po::value<int>()->default_value(1), "Parameter for ODS")
    ("sort_type,ST", po::value<int>()->default_value(1), "Selections for sorting type: 0: ODSTight, 1: ODSLoose, 2: bucketOSort, 3: bitonicSort, 4: mergeSort")
    ("datatype,DT", po::value<int>()->default_value(4), "#bytes for this kind of datatype, normally int32_t or int64_t");
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help") || vm.count("H")) {
      cout << desc << endl;
      exit(0);
    }
  } catch (exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Exception of unknown type! \n";
    exit(-1);
  }
  return vm;
}

void readParams(InputType inputtype, int &datatype, int64_t &N, int64_t &M, int &B, int &sigma, int &sortId, double &alpha, double &beta, double &gamma, int &P, int &argc, const char* argv[]) {
  if (inputtype == BOOST) {
    auto vm = read_options(argc, argv);
    datatype = vm["datatype"].as<int>();
    M = ((vm["memory"].as<int64_t>()) << 20) / datatype;
    N = vm["c"].as<int>() * M;
    B = vm["block_size"].as<int>();
    sigma = vm["sigma"].as<int>();
    sortId = vm["sort_type"].as<int>();
    alpha = vm["alpha"].as<double>();
    beta = vm["beta"].as<double>();
    gamma = vm["gamma"].as<double>();
    P = vm["P"].as<int>();
  } else if (inputtype == SETINMAIN) {
    datatype = 4;
    M = (128 << 20) / 32; // (MB << 20) / 1 element bytes
    N = 16 * M;
    B = 4; // (4 << 10) / 32; // 4KB pagesize
    sigma = 40;
    sortId = 1;
    alpha = 0.036152;
    beta = 0.037094;
    gamma = 0.037094;
    P = 18;
  }
}

int main(int argc, const char* argv[]) {
  int ret = 1;
  int *resId = new int;
  int *resN = new int;
  int FAN_OUT, BUCKET_SIZE;
  int inputId = 1;
  // oe_enclave_t* enclave = NULL;
  printf("Starting\n");
  high_resolution_clock::time_point start, end;
  milliseconds duration;
  // step1: init test numbers
  InputType inputtype = SETINMAIN;
  int datatype, B, sigma, sortId, P;
  int64_t N, M;
  double alpha, beta, gamma;
  readParams(inputtype, datatype, N, M, B, sigma, sortId, alpha, beta, gamma, P, argc, argv);
  printf("After read params\n");
  double params[10] = {(double)sortId, (double)inputId, (double)N, (double)M, (double)B, (double)sigma, alpha, beta, gamma, (double)P};
  // step2: Create the enclave
  // result = oe_create_oqsort_enclave(argv[1], OE_ENCLAVE_TYPE_SGX, OE_ENCLAVE_FLAG_DEBUG, NULL, 0, &enclave);
  // transition_using_threads
  /*
  oe_enclave_setting_context_switchless_t switchless_setting = {
        1,  // number of host worker threads
        1}; // number of enclave worker threads.
  oe_enclave_setting_t settings[] = {{
        .setting_type = OE_ENCLAVE_SETTING_CONTEXT_SWITCHLESS,
        .u.context_switchless_setting = &switchless_setting,
    }};
  result = oe_create_oqsort_enclave(argv[1], OE_ENCLAVE_TYPE_SGX, 0, settings, OE_COUNTOF(settings), &enclave);
  */
  // result = oe_create_oqsort_enclave(argv[1], OE_ENCLAVE_TYPE_SGX, 0, NULL, 0, &enclave);
  // 0: OQSORT-Tight, 1: OQSORT-Loose, 2: bucketOSort, 3: bitonicSort
  if (sortId == 3 && (N % B) != 0) {
    int64_t addi = addi = ((N / B) + 1) * B - N;
    N += addi;
  }
  DataStore data(arrayAddr, N, M, B);
  start = high_resolution_clock::now();
  if (sortId == 2) {
    int64_t totalSize = calBucketSize(sigma, N, M, B);
    data.init(inputId, N);
    data.init(inputId + 1, totalSize);
    data.init(inputId + 2, totalSize);
  } else {
    data.init(inputId, N);
  }
  callSort(resId, resN, params);
  end = high_resolution_clock::now();
  /*
  if (result != OE_OK) {
    fprintf(stderr, "Calling into enclave_hello failed: result=%u (%s)\n", result, oe_result_str(result));
    ret = -1;
  }*/
  // step4: std::cout execution time
  duration = duration_cast<milliseconds>(end - start);
  std::cout << "Time taken by sorting function: " << duration.count() << " miliseconds" << std::endl;
  printf("IOcost: %f, %f\n", IOcost/N*B, IOcost);
  // testEnc(arrayAddr, *resId, *resN);
  data.print(*resId, *resN, FILEOUT, data.filepath);
  // step5: exix part
  exit:
    /*
    if (enclave) {
      oe_terminate_enclave(enclave);
    }
    */
    delete resId;
    delete resN;
    return ret;
}