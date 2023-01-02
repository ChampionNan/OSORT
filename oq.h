#ifndef OQ_SORT_H
#define OQ_SORT_H

#include "shared.h"
#include <utility>

class ODS {
  public:
    ODS(EnclaveServer &eServer, SortType sorttype, int inputId, double alpha, double beta, double gamma, int P, int is_tight);
    void floydSampler(int64_t n, int64_t k, std::vector<int64_t> &x);
    int64_t Sample(int inStructureId, int64_t sampleSize, std::vector<EncOneBlock> &trustedM2);
    void quantileCal(std::vector<EncOneBlock> &samples, int64_t start, int64_t end, int p);
    int64_t partitionMulti(EncOneBlock *arr, int64_t low, int64_t high, EncOneBlock pivot);
    void quickSortMulti(EncOneBlock *arr, int64_t low, int64_t high, std::vector<EncOneBlock> pivots, int left, int right, std::vector<int64_t> &partitionIdx);
    std::pair<int64_t, int> OneLevelPartition(int inStructureId, int64_t inSize, std::vector<EncOneBlock> &samples, int64_t sampleSize, int p, int ouputId1);
    void ObliviousSort(int64_t inSize);

  public:
    int resultId;
    int64_t resultN;

  private:
    EnclaveServer eServer;
    int64_t N, M;
    int B;
    int is_rec, is_tight;
    int inputId, outputId1, outputId2;
    double alpha, beta, gamma;
    int P;
    SortType sorttype;
    std::random_device rd;
    std::mt19937 rng{rd()};
};

#endif // !OQ_SORT_H