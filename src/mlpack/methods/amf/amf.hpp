/**
 * @file amf.hpp
 * @author Sumedh Ghaisas
 * @author Mohan Rajendran
 * @author Ryan Curtin
 *
 * Alternating Matrix Factorization
 *
 * The AMF (alternating matrix factorization) class, from which more commonly
 * known techniques such as incremental SVD, NMF, and batch-learning SVD can be
 * derived.
 */
#ifndef __MLPACK_METHODS_AMF_AMF_HPP
#define __MLPACK_METHODS_AMF_AMF_HPP

#include <mlpack/core.hpp>

#include <mlpack/methods/amf/update_rules/nmf_mult_dist.hpp>
#include <mlpack/methods/amf/update_rules/nmf_als.hpp>
#include <mlpack/methods/amf/update_rules/svd_batch_learning.hpp>
#include <mlpack/methods/amf/update_rules/svd_incomplete_incremental_learning.hpp>
#include <mlpack/methods/amf/update_rules/svd_complete_incremental_learning.hpp>

#include <mlpack/methods/amf/init_rules/random_init.hpp>

#include <mlpack/methods/amf/termination_policies/simple_residue_termination.hpp>
#include <mlpack/methods/amf/termination_policies/simple_tolerance_termination.hpp>

namespace mlpack {
namespace amf /** Alternating Matrix Factorization **/ {

/**
 * This class implements AMF (alternating matrix factorization) on the given
 * matrix V. Alternating matrix factorization decomposes V in the form
 * \f$ V \approx WH \f$ where W is called the basis matrix and H is called the
 * encoding matrix. V is taken to be of size n x m and the obtained W is n x r
 * and H is r x m. The size r is called the rank of the factorization.
 *
 * The implementation requires three template types; the first contains the
 * policy used to determine when the algorithm has converged; the second
 * contains the initialization rule for the W and H matrix; the last contains
 * the update rule to be used during each iteration. This templatization allows
 * the user to try various update rules, initialization rules, and termination
 * policies (including ones not supplied with MLPACK) for factorization.  By
 * default, the template parameters to AMF implement non-negative matrix
 * factorization with the multiplicative distance update.
 *
 * A simple example of how to run AMF (or NMF) is shown below.
 *
 * @code
 * extern arma::mat V; // Matrix that we want to perform LMF on.
 * size_t r = 10; // Rank of decomposition
 * arma::mat W; // Basis matrix
 * arma::mat H; // Encoding matrix
 *
 * AMF<> amf; // Default options: NMF with multiplicative distance update rules.
 * amf.Apply(V, W, H, r);
 * @endcode
 *
 * @tparam TerminationPolicy The policy to use for determining when the
 *     factorization has converged.
 * @tparam InitializationRule The initialization rule for initializing W and H
 *     matrix.
 * @tparam UpdateRule The update rule for calculating W and H matrix at each
 *     iteration.
 *
 * @see NMFMultiplicativeDistanceUpdate, SimpleResidueTermination
 */
template<typename TerminationPolicyType = SimpleResidueTermination,
         typename InitializationRuleType = RandomInitialization,
         typename UpdateRuleType = NMFMultiplicativeDistanceUpdate>
class AMF
{
 public:
  /**
   * Create the AMF object and (optionally) set the parameters which AMF will
   * run with.  The minimum residue refers to the root mean square of the
   * difference between two subsequent iterations of the product W * H.  A low
   * residue indicates that subsequent iterations are not producing much change
   * in W and H.  Once the residue goes below the specified minimum residue, the
   * algorithm terminates.
   *
   * @param initializationRule Optional instantiated InitializationRule object
   *      for initializing the W and H matrices.
   * @param updateRule Optional instantiated UpdateRule object; this parameter
   *      is useful when the update rule for the W and H vector has state that
   *      it needs to store (i.e. HUpdate() and WUpdate() are not static
   *      functions).
   * @param terminationPolicy Optional instantiated TerminationPolicy object.
   */
  AMF(const TerminationPolicyType& terminationPolicy = TerminationPolicyType(),
      const InitializationRuleType& initializeRule = InitializationRuleType(),
      const UpdateRuleType& update = UpdateRuleType());

  /**
   * Apply Alternating Matrix Factorization to the provided matrix.
   *
   * @param V Input matrix to be factorized.
   * @param W Basis matrix to be output.
   * @param H Encoding matrix to output.
   * @param r Rank r of the factorization.
   */
  template<typename MatType>
  double Apply(const MatType& V,
               const size_t r,
               arma::mat& W,
               arma::mat& H);

  //! Access the termination policy.
  const TerminationPolicyType& TerminationPolicy() const
  { return terminationPolicy; }
  //! Modify the termination policy.
  TerminationPolicyType& TerminationPolicy() { return terminationPolicy; }

  //! Access the initialization rule.
  const InitializationRuleType& InitializeRule() const
  { return initializationRule; }
  //! Modify the initialization rule.
  InitializationRuleType& InitializeRule() { return initializationRule; }

  //! Access the update rule.
  const UpdateRuleType& Update() const { return update; }
  //! Modify the update rule.
  UpdateRuleType& Update() { return update; }

 private:
  //! Termination policy.
  TerminationPolicyType terminationPolicy;
  //! Instantiated initialization Rule.
  InitializationRuleType initializationRule;
  //! Instantiated update rule.
  UpdateRuleType update;
}; // class AMF

typedef amf::AMF<amf::SimpleResidueTermination,
                 amf::RandomInitialization, 
                 amf::NMFALSUpdate> NMFALSFactorizer;

//! Add simple typedefs 
#ifdef MLPACK_USE_CXX11

/**
 * SVDBatchFactorizer factorizes given matrix V into two matrices W and H by
 * gradient descent. SVD batch learning is described in paper 'A Guide to 
 * singular Value Decomposition' by Chih-Chao Ma.
 *
 * @see SVDBatchLearning
 */
template<class MatType>
using SVDBatchFactorizer = amf::AMF<amf::SimpleToleranceTermination<MatType>,
                                    amf::RandomInitialization,
                                    amf::SVDBatchLearning>;

/**
 * SVDIncompleteIncrementalFactorizer factorizes given matrix V into two matrices 
 * W and H by incomplete incremental gradient descent. SVD incomplete incremental 
 * learning is described in paper 'A Guide to singular Value Decomposition' 
 * by Chih-Chao Ma.
 *
 * @see SVDIncompleteIncrementalLearning
 */                                    
template<class MatType>
using SVDIncompleteIncrementalFactorizer = amf::AMF<amf::SimpleToleranceTermination<MatType>,
                                                    amf::RandomInitialization,
                                                    amf::SVDIncompleteIncrementalLearning>;
/**
 * SVDCompleteIncrementalFactorizer factorizes given matrix V into two matrices 
 * W and H by complete incremental gradient descent. SVD complete incremental 
 * learning is described in paper 'A Guide to singular Value Decomposition' 
 * by Chih-Chao Ma.
 *
 * @see SVDCompleteIncrementalLearning
 */                                                        
template<class MatType>
using SVDCompleteIncrementalFactorizer = amf::AMF<amf::SimpleToleranceTermination<MatType>,
                                                  amf::RandomInitialization,
                                                  amf::SVDCompleteIncrementalLearning<MatType> >;

#else // #ifdef MLPACK_USE_CXX11

/**
 * SparseSVDBatchFactorizer factorizes given sparse matrix V into two matrices 
 * W and H by gradient descent. SVD batch learning is described in paper 'A Guide to 
 * singular Value Decomposition' by Chih-Chao Ma. 
 *
 * @see SVDBatchLearning
 */              
typedef amf::AMF<amf::SimpleToleranceTermination<arma::sp_mat>,
                 amf::RandomInitialization,
                 amf::SVDBatchLearning> SparseSVDBatchFactorizer;

/**
 * SparseSVDBatchFactorizer factorizes given matrix V into two matrices 
 * W and H by gradient descent. SVD batch learning is described in paper 'A Guide to 
 * singular Value Decomposition' by Chih-Chao Ma. 
 *
 * @see SVDBatchLearning
 */             
typedef amf::AMF<amf::SimpleToleranceTermination<arma::mat>,
                 amf::RandomInitialization,
                 amf::SVDBatchLearning> SVDBatchFactorizer;
/**
 * SparseSVDIncompleteIncrementalFactorizer factorizes given sparse matrix V 
 * into two matrices W and H by incomplete incremental gradient descent. 
 * SVD incomplete incremental learning is described in paper 'A Guide to singular 
 * Value Decomposition' by Chih-Chao Ma.
 *
 * @see SVDIncompleteIncrementalLearning
 */                   
typedef amf::AMF<amf::SimpleToleranceTermination<arma::sp_mat>,
                 amf::RandomInitialization,
                 amf::SVDIncompleteIncrementalLearning> 
        SparseSVDIncompleteIncrementalFactorizer;

/**
 * SVDIncompleteIncrementalFactorizer factorizes given matrix V into two matrices 
 * W and H by incomplete incremental gradient descent. SVD incomplete incremental 
 * learning is described in paper 'A Guide to singular Value Decomposition' 
 * by Chih-Chao Ma.
 *
 * @see SVDIncompleteIncrementalLearning
 */              
typedef amf::AMF<amf::SimpleToleranceTermination<arma::mat>,
                 amf::RandomInitialization,
                 amf::SVDIncompleteIncrementalLearning> 
        SVDIncompleteIncrementalFactorizer;

/**
 * SparseSVDCompleteIncrementalFactorizer factorizes given sparse matrix V 
 * into two matrices W and H by complete incremental gradient descent. SVD 
 * complete incremental learning is described in paper 'A Guide to singular 
 * Value Decomposition' by Chih-Chao Ma.
 *
 * @see SVDCompleteIncrementalLearning
 */   
typedef amf::AMF<amf::SimpleToleranceTermination<arma::sp_mat>,
                 amf::RandomInitialization,
                 amf::SVDCompleteIncrementalLearning<arma::sp_mat> > 
        SparseSVDCompleteIncrementalFactorizer;

/**
 * SVDCompleteIncrementalFactorizer factorizes given matrix V into two matrices 
 * W and H by complete incremental gradient descent. SVD complete incremental 
 * learning is described in paper 'A Guide to singular Value Decomposition' 
 * by Chih-Chao Ma.
 *
 * @see SVDCompleteIncrementalLearning
 */                      
typedef amf::AMF<amf::SimpleToleranceTermination<arma::mat>,
                 amf::RandomInitialization,
                 amf::SVDCompleteIncrementalLearning<arma::mat> > 
        SVDCompleteIncrementalFactorizer;

#endif // #ifdef MLPACK_USE_CXX11


}; // namespace amf
}; // namespace mlpack

// Include implementation.
#include "amf_impl.hpp"

#endif // __MLPACK_METHODS_AMF_AMF_HPP

