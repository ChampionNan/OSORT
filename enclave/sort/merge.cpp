#include "merge.h"
#include "bucket.h"

int merge_sort(int inStructureId, int outStructureId, int size) {
  int bucketNum = ceil(1.0 * size / M);
  printf("Bucket Number: %d", bucketNum);
  int setSize = floor(1.0 * M / (bucketNum + 1));
  initMerge(size);
  printf("HEAP_NODE_SIZE: %d\n", setSize);
  int avg = size / bucketNum;
  int remainder = size % bucketNum;
  int *bucketAddr = (int*)malloc(bucketNum * sizeof(int));
  int *elements = (int*)malloc(bucketNum * sizeof(int));
  int each, total = 0;
  for (int i = 0; i < bucketNum; ++i) {
    each = avg + ((i < remainder) ? 1 : 0);
    elements[i] = each;
    bucketAddr[i] = total;
    total += each;
  }
  Bucket_x *arr = (Bucket_x*)malloc(M * sizeof(Bucket_x));
  for (int i = 0; i < bucketNum; ++i) {
    bucketSort(inStructureId, elements[i], bucketAddr[i]);
  }
  kWayMergeSort(inStructureId, outStructureId, elements, bucketAddr, bucketNum);
  return outStructureId;
}