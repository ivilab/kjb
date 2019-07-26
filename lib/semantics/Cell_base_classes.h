/* $Id: Cell_base_classes.h 17119 2014-07-17 00:46:17Z cdawson $ */

#ifndef CELL_BASE_CLASSES_H_
#define CELL_BASE_CLASSES_H_

/*!
 * @file Cell_base_classes.h
 *
 * @author Colin Dawson 
 */

#include <semantics/Categorical_event.h>
#include <boost/shared_ptr.hpp>
#include <vector>

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

namespace semantics
{
    class Cell;
    class Marginal_cell_base;
    class Context_cell_base;

class Cell
{
public:
    typedef boost::shared_ptr<Cell> Self_ptr;
    typedef std::vector<size_t> Step_sizes;
    static bool VERBOSE;
public:
    /*! @brief get current count in this cell
     */
    const size_t& count() const {return count_;}
    
    /*! @brief get number of "tables" in this cell
     */
    const size_t& table_count() const {return table_count_;}
    
    /*! @brief return variable values in this cell
     */
    virtual const Categorical_event_base& key() const = 0;
    
    /*! @brief increment count
     */
    virtual void increment() {++count_;}

    /*! @brief decrement count
     */
    virtual void decrement()
    {
        assert(count_ > 0);
        --count_;
    }

    /*! @brief increment table count
     */
    virtual void increment_table_count() {++table_count_;}

    /*! @brief decrement table count
     */
    virtual void decrement_table_count()
    {
        assert(table_count_ > 0);
        --table_count_;
    }

protected:
    size_t count_;
    size_t table_count_;
    
    /*! @brief default ctor
     */
    Cell() : count_(0), table_count_(0) {}

    /*! @brief virtual destructor
     */
    virtual ~Cell() {}
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

class Marginal_cell_base : public Cell
{
public:
    typedef boost::shared_ptr<Marginal_cell_base> Self_ptr;
    typedef boost::shared_ptr<Context_cell_base> Context_ptr;
    typedef std::map<size_t, size_t> Table_count_map;
protected:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/

    /*! @brief constructor
     */
    Marginal_cell_base()
        : Cell(),
          per_table_counts_()
    {}

public:

    /*------------------------------------------------------------
     * ACCCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief get smoothed marginal cell ptr
     */
    virtual Self_ptr margin() const = 0;

    /*! @brief get ptr to conditioning context cell
     */
    virtual Context_ptr context() const = 0;

    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/

    /*! @brief increment the count associated with a particular table
     */
    virtual void increment(const size_t& table_code) = 0;
    
    /*! @brief increment the count associated with a particular table
     */
    virtual void decrement(const size_t& table_code) = 0;
    
    /*! @brief sample from the conditional predictive distribution of table assignment
     */
    virtual size_t sample_table_assignment() = 0;

    /*------------------------------------------------------------
     * COMPUTATION
     *------------------------------------------------------------*/

    /*! @brief get smoothed probability estimate
     */
    virtual double smoothed_probability() = 0;

    /*! @brief prob of this outcome conditioned on starting a new table
     *
     *  corresponds to q(x,H) in the writeup
     */
    virtual double backoff_probability() const = 0;

    /*! @brief predictive probability of this outcome in this context
     *
     * corresponds to p(x|beta(H)) in the writeup, but is not normalized.
     * normalization must occur later when comparing
     */
    virtual double predictive_probability() const = 0;
protected:
    Table_count_map per_table_counts_;
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

class Context_cell_base : public Cell
{
public:
    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    typedef boost::shared_ptr<Context_cell_base> Self_ptr;

    /*------------------------------------------------------------
     * STATIC MEMBERS
     *------------------------------------------------------------*/

    //// Gamma hyperparameters for concentration parameter alpha
    static const double& a();
    static const double& b();
    
protected:
    
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief constructor
     */
    Context_cell_base()
        : Cell(),
          alpha_(1.0),
          w_(0.5),
          s_(0.0),
          diversity_(0)
    {}

public:

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief get number of distinct outcomes in this context
     */
    int diversity() const {return diversity_;}

    /*! @brief get the Dirichlet Process concentration parameter for this context
     */
    double alpha() const {return alpha_;}

    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/

    /*! @brief increment the number of distinct outcomes from this context
     */
    virtual void increment_diversity() {++diversity_;}

    /*! @brief decrement the number of distinct outcomes from this context
     */
    virtual void decrement_diversity() {--diversity_;}

    /*! @brief resample alpha value from conditional posterior
     */
    void resample_alpha();

    /*------------------------------------------------------------
     * COMPUTATION
     *------------------------------------------------------------*/

    /*! @brief total mass for distribution over the "dish" for a new table
     *
     *  corresponds to alpha + m(H)
     */
    double effective_tables() const {return alpha_ + table_count_;}

    /*! @brief total mass for distribution over dish for a new customer
     *
     *  corresponds to alpha + n(H)
     */
    double effective_observations() const {return alpha_ + count_;}
    
    /*! @brief probability that a new table will draw from the level above this
     *
     *  corresponds to mixture weight (alpha / (m + alpha))
     */
    double table_backoff_weight() const {return alpha_ / effective_tables();}

    /*! @brief probability that a new customer will sit at a new table
     *
     * corresponds to mixture weight (alpha / (n + alpha))
     */
    double observation_backoff_weight() const
    {
        return alpha_ / effective_observations();
    }

private:
    double alpha_;
    double w_;
    double s_;
    size_t diversity_;
};

}; //namespace semantics


#endif
