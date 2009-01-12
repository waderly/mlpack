#ifndef SUM_H
#define SUM_H

#include "fastlib/fastlib.h"
#include "operator.h"

class Sum: public Operator {

 public:

  double NaiveCompute
  (std::map<index_t, index_t> &constant_dataset_indices) {

    double sum_result = 0;
 
    // Get the list of restrictions associated with the current
    // dataset index.
    std::map<index_t, std::vector<int> >::iterator restriction = 
      restrictions_->find(dataset_index_);
  
    // The current dataset that is involved.
    const Matrix *dataset = (*datasets_)[dataset_index_];

    // Loop over all indices of the current operator.
    for(index_t n = 0; n < dataset->n_cols(); n++) {

      if(restriction != restrictions_->end() &&
	 CheckViolation_(constant_dataset_indices, (*restriction).second, n)) {
	continue;
      }

      // Re-assign the point index for the current dataset.
      constant_dataset_indices[dataset_index_] = n;

      // Recursively evaluate the operators and add them up.
      for(index_t i = 0; i < operators_.size(); i++) {
	sum_result += operators_[i]->NaiveCompute(constant_dataset_indices);
      }      
    }
    return PostProcess_(constant_dataset_indices, sum_result);
  }

  double MonteCarloCompute
  (std::map<index_t, index_t> &constant_dataset_indices) {

    double sum_result = 0;

    // Sample over some combinations of the current operator, and
    // recursively call the operators underneath the current operator,
    // and summing all of them up.
    int sample_size = 50;
    for(index_t s = 0; s < sample_size; s++) {

      ChoosePointIndex_(constant_dataset_indices);
      
      // Recursively evaluate the operators and add them up.
      for(index_t i = 0; i < operators_.size(); i++) {
	sum_result += operators_[i]->MonteCarloCompute
	  (constant_dataset_indices);
      }
    }
    
    // Compute the sample average and multiply by the number of terms
    // in the current dataset index.
    sum_result = sum_result / ((double) sample_size) * 
      (((*datasets_)[dataset_index_])->n_cols());
    
    return PostProcess_(constant_dataset_indices, sum_result);
  }

};

#endif
