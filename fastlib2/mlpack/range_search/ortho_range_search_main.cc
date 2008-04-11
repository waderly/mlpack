/** @file ortho_range_search_main.cc
 *
 *  A test driver for the orthogonal range search code.
 *
 *  @author Dongryeol Lee (dongryel)
 */
#include "data_aux.h"
#include "naive_ortho_range_search.h"
#include "ortho_range_search.h"

#include "fastlib/mmanager/memory_manager.h"

/** Main function which reads parameters and runs the orthogonal
 *  range search.
 *
 *  In order to compile this driver, do:
 *  fl-build ortho_range_search_bin --mode=fast
 *
 *  Type the following (which consists of both required and optional
 *  arguments) in a single command line:
 *
 *  ./ortho_range_search_bin --data=dataset
 *                           --range=range
 *                           --do_naive
 *                           --save_tree_file=savedtree
 *                           --load_tree_file=savedtree
 *
 *  Explanations for the arguments listed with possible values.
 *
 *  1. data (required): the name of the dataset.
 *
 *  2. range (required): the textfile containing the range. The following
 *                       is an example:
 *
 *                       0.88 INFINITY
 *
 *                       0.90 INFINITY
 *
 *                       0.90 INFINITY
 *
 *                       0.90 INFINITY
 *
 *                       for 4-dimensional search window of
 *                       [0.88,inf]*[0.90,inf]*[0.90,inf]*[0.90,inf]
 *
 *  3. do_naive (optional): if present, will run the naive algorithm to
 *                          compare the search result against the naive search.
 *  4. save_tree_file (optional): this feature is in a beta stage and WILL NOT
 *                                work with the current FastLib release. It 
 *                                saves the constructed tree in the fast 
 *                                algorithm to a file, so that it can be loaded
 *                                later via --load_tree_file argument. If you 
 *                                want to use this, let me 
 *                                (dongryel@cc.gatech.edu) know.
 *  5. load_tree_file (optional): this feature is in a beta stage and WILL NOT
 *                                work with the current FastLib release. It
 *                                loads the saved tree from a file, so that
 *                                the algorithm does not have to construct
 *                                trees. If you want to use this feature, let
 *                                me (dongryel@cc.gatech.edu) know.
 */
int main(int argc, char *argv[]) {

  // Initialize Nick's memory manager...
  std::string pool_name("/scratch/temp_mem");
  mmapmm::MemoryManager<false>::allocator_ = 
    new mmapmm::MemoryManager<false>();
  mmapmm::MemoryManager<false>::allocator_->set_pool_name(pool_name);
  mmapmm::MemoryManager<false>::allocator_->set_capacity(65536*1000);
  mmapmm::MemoryManager<false>::allocator_->Init();

  // Always initialize FASTexec with main's inputs at the beggining of
  // your program.  This reads the command line, among other things.
  fx_init(argc, argv);

  ////////// READING PARAMETERS AND LOADING DATA /////////////////////

  // The reference data file is a required parameter.
  const char* dataset_file_name = fx_param_str(NULL, "data", "data.ds");

  // column-oriented dataset matrix.
  GenMatrix<short int> dataset;

  // data::Load inits a matrix with the contents of a .csv or .arff.
  data_aux::Load(dataset_file_name, &dataset);

  // Read the search range from the file. Note that the ranges will be
  // a two-column dataset: the first column denotes the lower limits
  // and the second column denotes the upper limits.
  const char *range_data_file_name = fx_param_str(NULL, "range", "range.ds");
  GenVector<short int> low_coord_limits, high_coord_limits;
  GenMatrix<short int> range_dataset;
  data_aux::LoadTranspose(range_data_file_name, &range_dataset);
  range_dataset.MakeColumnVector(0, &low_coord_limits);
  range_dataset.MakeColumnVector(1, &high_coord_limits);

  // flag for determining whether we need to do naive algorithm.
  bool do_naive = fx_param_exists(NULL, "do_naive");

  // File name containing the saved tree (if the user desires to do so)
  const char *load_tree_file_name;

  if(fx_param_exists(NULL, "load_tree_file")) {
    load_tree_file_name = fx_param_str(NULL, "load_tree_file", NULL);
  }
  else {
    load_tree_file_name = NULL;
  }

  // Declare fast tree-based orthogonal range search algorithm object.
  OrthoRangeSearch<short int> fast_search;
  fast_search.Init(dataset, fx_param_exists(NULL, "do_naive"),
		   load_tree_file_name);
  fast_search.Compute(low_coord_limits, high_coord_limits);

  if(fx_param_exists(NULL, "save_tree_file")) {
    const char *save_tree_file_name = fx_param_str(NULL, "save_tree_file",
						   NULL);
    fast_search.SaveTree(save_tree_file_name);
  }

  ArrayList<bool> fast_search_results;
  fast_search.get_results(&fast_search_results);

  // if naive option is specified, do naive algorithm
  if(do_naive) {
    NaiveOrthoRangeSearch<short int> search;
    search.Init(dataset);
    search.Compute(low_coord_limits, high_coord_limits);
    ArrayList<bool> naive_search_results;
    search.get_results(&naive_search_results);
    bool flag = true;

    for(index_t i = 0; i < fast_search_results.size(); i++) {

      if(fast_search_results[i] != naive_search_results[i]) {
	flag = false;
	printf("Differ on %d\n", i);
	break;
      }
    }
    if(flag) {
      printf("Both naive and tree-based method got the same results...\n");
    }
    else {
      printf("Both methods have different results...\n");
    }
  }
  timer *tree_range_search_time = fx_timer(NULL, "tree_range_search");
  timer *naive_search_time = fx_timer(NULL, "naive_search");
	 
  fx_format_result(NULL, "speedup", "%g", 
		   ((double)(naive_search_time->total).micros) / 
		   ((double)(tree_range_search_time->total).micros));
  fx_done();

  // Destroy Nick's memory manager...
  mmapmm::MemoryManager<false>::allocator_->Destruct();
  delete mmapmm::MemoryManager<false>::allocator_;
  return 0;
}
