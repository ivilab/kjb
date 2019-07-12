/* $Id: sample_recorder.h 12975 2012-09-11 23:15:57Z ksimek $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek
 * =========================================================================== */

#ifndef SAMPLE_RECORDER_H
#define SAMPLE_RECORDER_H

#include <limits>
#include <deque>
#include <vector>
#include <ostream>
#include <string>

#include <boost/function.hpp>
#include <boost/concept_check.hpp>
#include <boost/concept_archetype.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>
#include <boost/bind/apply.hpp>
#include <boost/any.hpp>
#include <boost/concept_check.hpp>

//#include <l_cpp/l_cpp_incl.h>
#include <l/l_sys_io.h>

#include "sample_cpp/sample_concept.h"
#include "sample_cpp/sample_base.h"
#include <l_cpp/l_serialization.h>

struct Null_value{ };
/**
 * @class Current_model_recorder
 *
 * Implements the ModelRecorder concept.
 *
 * Keeps track of the best model; ignores the rest.
 *
 * TODO: Split into Current_model_recorder and Current_log_recorder.
 */
template <class Model>
class Current_model_recorder
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef Model Value_type;

    Current_model_recorder() :
        m_current_model(),
        m_current_lt(-std::numeric_limits<double>::max())
    {}

    Current_model_recorder(const Model& initial_model) :
        m_current_model(initial_model),
        m_current_lt(-std::numeric_limits<double>::max())
    {}

    void operator()(const Model& m, const Step_log<Model>& step_log)
    {
        assert(step_log.size() >= 1);

        // final entry of the log contains most recent log-target value
        //Step_result& result = *(step_log.end()--);
        const Step_result<Model>& result = step_log.back();

        m_current_model = m;
        m_current_lt = result.lt;
    }

    const Value_type& get() const
    {
        if(m_current_lt == -std::numeric_limits<double>::max())
        {
            KJB_THROW_2(kjb::Runtime_error, "current model has not been recorded yet.");
        }

        return *m_current_model;
    }

private:
    boost::optional<Model> m_current_model;
    double m_current_lt;
};

template <class Model>
class Current_log_recorder
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef Step_log<Model> Value_type;

    Current_log_recorder() :
        m_current_log()
    {}

    void operator()(const Model& /*m*/, const Step_log<Model>& step_log)
    {
        assert(step_log.size() >= 1);

        m_current_log = step_log;
    }

    const Value_type& get() const
    {
        if(m_current_log.size() == 0)
        {
            KJB_THROW_2(kjb::Runtime_error, "current log has not been recorded yet.");
        }

        return m_current_log;
    }

private:
    Step_log<Model> m_current_log;
};

/**


 * @class Best_model_recorder
 *
 * Implements the ModelRecorder concept.
 *
 * Keeps track of the best model; ignores the rest.
 */
template <class Model>
class Best_model_recorder
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef Model Value_type;

    Best_model_recorder() :
        m_best_model(),
        m_best_lt(-std::numeric_limits<double>::max())
    {}

    Best_model_recorder(const Model& initial_model, double initial_lt) :
        m_best_model(initial_model),
        m_best_lt(initial_lt)
    {}

    void operator()(const Model& m, Step_log<Model> step_log)
    {

        assert(step_log.size() >= 1);

        // final entry of the log contains most recent log-target value
        Step_result<Model>& result = step_log.back();

        if(!m_best_model || result.lt > m_best_lt)
        {
            m_best_model = m;
            m_best_lt = result.lt;
        }

    }

    const Value_type& get() const
    {
        if(m_best_lt == -std::numeric_limits<double>::max())
        {
            KJB_THROW_2(kjb::Runtime_error, "Best model has not been recorded yet.");
        }

        return *m_best_model;
    }

    double get_log_target() const
    {
        return m_best_lt;
    }

private:
    boost::optional<Model> m_best_model;
    double m_best_lt;
};

/**
 * @class Best_target_recorder
 *
 * Implements the ModelRecorder concept.
 *
 * Keeps track of the best target distribution value; ignores the rest.
 */
template <class Model>
class Best_target_recorder
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef double Value_type;

    Best_target_recorder() :
        m_best_lt(-std::numeric_limits<double>::max())
    {}

    void operator()(const Model& /*m*/, Step_log<Model> step_log)
    {

        assert(step_log.size() >= 1);

        // final entry of the log contains most recent log-target value
        //Step_result<Model>& result = *(step_log.end()--);
        Step_result<Model>& result = step_log.back();

        if(result.lt > m_best_lt)
        {
            m_best_lt = result.lt;
        }
    }

    const Value_type& get() const
    {
        if(m_best_lt == -std::numeric_limits<double>::max())
        {
            KJB_THROW_2(kjb::Runtime_error, "Best model has not been recorded yet.");
        }

        return m_best_lt;
    }

private:
    double m_best_lt;
};

/**
 * @class All_model_recorder
 *
 * Implements the Model_recorder concept.
 *
 * Saves all models passed to it, but ignores the log.
 */
template <class Model>
class All_model_recorder : public std::vector<Model>
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef std::vector<Model> Value_type;


    All_model_recorder() :
        std::vector<Model>()
    {}

    void operator()(const Model& m, const Step_log<Model>& /*step_log*/)
    {
        this->push_back(m);
    }

    const Value_type& get() const { return *this; }
};


/**
 * @class All_log_recorder
 *
 * Implements the Model_recorder concept.
 *
 * Keeps all log entries, but ignores the models.
 */
template <class Model>
class All_log_recorder : public Step_log<Model>
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef Step_log<Model> Value_type;

    All_log_recorder() :
        Step_log<Model>()
    {}

    void operator()(const Model& /*m*/, const Step_log<Model>& step_log)
    {
        (*this) += step_log;
    }

    const Value_type& get() const { return *this; }
};


/**
 * class Recent_model_recorder
 *
 * Implements the Model_recorder concept.
 *
 * Saves the models in a queue, with an optional bound on the 
 * queue length.
 */
template <class Model>
class Recent_model_recorder : public std::deque<Model>
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef std::deque<Model> Value_type;
    typedef std::deque<Model> Parent;

    /**
     * @param n Maximum number of elements to store before deleting old elements.  If zero, all elements are saved.
     */
    Recent_model_recorder(int n = 0) :
        m_n(n)
    {}

    virtual ~Recent_model_recorder()
    {}

    virtual void operator()(const Model& m, const Step_log<Model>& /*step_log*/)
    {
        this->push_back(m);

        if(m_n > 0 && Parent::size() > m_n)
        {
            Parent::pop_front();
        }
    }

    const Value_type& get() const { return *this; }
private:
    size_t m_n;
};


/**
 * @class Recent_log_recorder
 *
 * Implements the Model_recorder concept.
 *
 * Keeps all log entries, but ignores the models.
 */
template <class Model>
class Recent_log_recorder : public Step_log<Model>
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef Step_log<Model> Value_type;

    Recent_log_recorder(size_t n = 0) :
        Step_log<Model>(),
        m_n(n)
    {}

    void operator()(const Model& /*m*/, const Step_log<Model>& step_log)
    {
        (*this) += step_log;

        if(m_n > 0 && this->size() > m_n)
        {
            this->pop_front();
        }

    }

    const Value_type& get() const
    {
        return *this;
    }
private:
    size_t m_n;
};



/**
 * @class Expectation_recorder
 *
 * Implements the Model_recorder concept.
 *
 * Approximates expected value of a function.
 */
template <class Model, class Value>
class Expectation_recorder
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model Model_type;
    typedef Value Value_type;

    typedef boost::function1<Value_type, const Model&> Func;

    Expectation_recorder(const Func& f, const Value_type& initial_val = Value_type()) :
        m_func(f),
        m_total(initial_val),
        m_n(0)
    {}

    virtual ~Expectation_recorder()
    {}

    virtual void operator()(const Model& m, const Step_log<Model>& /* step_log */)
    {
        m_total += m_func(m);
        m_n++;
    }

    const Value_type get() const
    {
        return m_total / m_n;
    }
private:
    Func m_func;
    Value_type m_total;
    size_t m_n;
};


/**
 * @class Multi_model_recorder
 *
 * Implements the Model_recorder concept.
 *
 * Collection of recorders that are called in sequence.
 *
 * @note This recorder's get() method is useless.  To get values from individual recorders, call the get<>(int) template method (see example code below).
 *
 * <code>
 *      typedef Best_model_recorder<Model> Best_recorder;
 *      typedef boost::shared_ptr<Best_recorder> Best_recorder_ptr;
 *      Multi_model_recorder<Model> recorders;
 *
 *   // Initialize multi-recorder
 *      // example 1: simple add
 *      Best_recorder r1;
 *      recorders.add(r1);
 *
 *      // example 2: inline add
 *      recorders.add( Best_recorder() );
 *
 *      // example 3: add by pointer
 *      Best_recorder_ptr r3(new Best_recorder());
 *      recorders.add(r3);
 *
 *      // example 4: add by reference 
 *      Best_recorder r4;
 *      recorders.add(boost::ref(r4));
 *
 *  // RUN SAMPLER
 *      Sampler sampler(..., recorders);
 *      sampler.run(1000);
 *
 *  // RETRIEVE RESULTS
 *      Model best;
 *
 *  // EXAMPLE 1: GET RECORDED VALUE DIRECTLY
 *      best = sampler.get_recorder().get<Best_recorder>(0);
 *
 *
 *  // EXAMPLE 2: GET RECORDER FIRST, THEN GET VALUES
 *      // This allows us to call the speciailized methods of the
 *      // recorder, e.g. Best_model_recorder::get_log_target().
 *
 *      Best_recorder& tmp = sampler.get_recorder().get_recorder<Best_recorder>(0);
 *      best = tmp.get();
 *      best_target = tmp.get_log_target();
 *
 *
 *  // EXAMPLE 3: using passed-in pointer/reference
 *      // Since r3 and r4 were passed by pointer and reference,
 *      // we can get results directly from them.   
 *
 *      best = r3->get();
 *      best = r4.get();
 *
 *      // Note that if we tried
 *      // this with r1, we wouldn't get anything, because it was
 *      // _copied_ into the multi-recorder, whereas r3 and r4 were
 *      // stored as references.
 * </code>
 */
template <class Model>
class Multi_model_recorder 
{
struct null_deleter {
    void operator()(void const * ) const {}
};

protected:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef Model_recorder<Model> Recorder;
//    typedef std::vector<typename Model_recorder<Model>::Type > Base;

    std::vector<typename Model_recorder<Model>::Type > record_callbacks_;

    // important that this is a list, because record_callbacks_
    // will store references to these, and we don't want them reallocated.
    std::vector<boost::any> recorders_;

    std::vector<bool> is_reference_;


public:
    typedef Model Model_type;
    typedef Null_value Value_type; // no get function

    Multi_model_recorder()
    {}

    virtual void operator()(const Model& m, const Step_log<Model>& step_log)
    {
        using namespace boost;
        // call all recorder callbacks, sending these parameters
//        for_each(record_callbacks_.begin(), record_callbacks_.end(), bind(apply<void>(), _1, m, step_log));
        for(size_t i = 0; i < record_callbacks_.size(); i++)
        {
            record_callbacks_[i](m, step_log);
        }
    }

    size_t size() const { return recorders_.size(); }

    /**
     * @warning DEPRECATED.  Use add() instead.
     */
    template <class RecorderIn>
    void push_back(const RecorderIn& r)
    {
        add(r);
    }

    /**
     * Add a Recorder by value.  Recorder is copied.
     */
    template <class RecorderIn>
    void add(const RecorderIn& r_in)
    {
        BOOST_CONCEPT_ASSERT((boost::CopyConstructible<RecorderIn>));
        BOOST_CONCEPT_ASSERT((ModelRecorder<RecorderIn>));

        // we MUST store recorders as pointers.  We tried
        // storing as values, but copying a Multi_model_recorder 
        // breaks the references in recorder_callbacks_, and
        // it's impossible to fix, because by the time we copy,
        // we don't know the types stored in the recorders_ array.
        //
        // Shared pointers solves this problem and makes copying straightforward.
        boost::shared_ptr<RecorderIn> r(new RecorderIn(r_in));
        add(r);

    }

    /**
     * Add a Recorder by reference.
     */
    template <class RecorderIn>
    void add(const typename boost::reference_wrapper<RecorderIn>& r)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<RecorderIn>));

        // value stored in recorders_ is a boost::ref (special case)
        is_reference_.push_back(true);
        recorders_.push_back(r);
        record_callbacks_.push_back(r);
    }

    /**
     * Add a Recorder using a smart pointer.
     */
    template <class RecorderIn>
    void add(const typename boost::shared_ptr<RecorderIn>& r)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<RecorderIn>));

        // value stored in recorders_ is a boost::shared_ptr (Defualt)
        is_reference_.push_back(false);
        // copy recorder into generic array
        recorders_.push_back(r);
        // store operator() as callback so we don't need to know the type later on.
        // convert pointer to a reference, then store
        record_callbacks_.push_back(boost::ref(*r));
    }

    /**
     * Add a Recorder using a smart pointer.
     */
    template <class RecorderIn>
    void add(RecorderIn* r)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<RecorderIn>));
        boost::shared_ptr<RecorderIn> r_sptr(r, null_deleter());
        add(r_sptr);
    }


    /// Useless: returns void.  Call the original recorders' get() methods directly.
    const Value_type& get() const {static Value_type a; return a;}

    template <class RecorderOut>
    const RecorderOut& get_recorder(size_t i) const
    {
        
        typedef typename boost::shared_ptr<RecorderOut> RecorderPtr;
        typedef typename boost::reference_wrapper<RecorderOut> RecorderRef;

        if(is_reference_[i])
        {
            const RecorderRef result = boost::any_cast<RecorderRef>(recorders_[i]);
            return result;
        }
        else
        {
            const RecorderPtr result = boost::any_cast<RecorderPtr>(recorders_[i]);
            return *result;
        }

    }

    template <class RecorderOut>
    const typename RecorderOut::Value_type& get(size_t i) const
    {
        typedef typename boost::shared_ptr<RecorderOut> RecorderPtr;
        typedef typename boost::reference_wrapper<RecorderOut> RecorderRef;

        if(is_reference_[i])
        {
            const RecorderRef result = boost::any_cast<RecorderRef>(recorders_[i]);
            return static_cast<const RecorderOut&>(result).get();
            
        }
        else
        {
            const RecorderPtr result = boost::any_cast<RecorderPtr>(recorders_[i]);
            return result->get();
        }
    }
};

/**
 * "Wraps" and existing recorder, making it only record every Nth entry.  Results are still obtained through the wrapped recorder, not this object.
 *
 * @tparam Recorder A type implemeting the ModelRecorder concept.
 *
 * @warning The wrapped recorder is stored by reference, so the caller must ensure it isn't freed while Modulo_recorder is in use.
 */
template <class Recorder>
class Modulo_recorder
{
public:
    BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));;

    typedef typename Recorder::Model_type Model_type;
    typedef typename Recorder::Value_type Value_type;

    /**
     * @param recorder  The recorder to wrap.  This will be stored be value, so the caller is responsible for making sure the recorder remains in scope
     * @param interval  One in every _interval_ entries will be recorded
     * @param start     Where to start the counter.  When counter % interval == 0, the entry
     * will be recorded.  By default, the first entry is recorded.
     *
     *
     */
    Modulo_recorder(const Recorder& recorder, int interval, int start = 0) :
        r(recorder),
        N(interval),
        n(start)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));
    }

    virtual void operator()(const Model_type& m, const Step_log<Model_type>& step_log)
    {
        if(n++ % N == 0)
        {
            // record the entry
            r(m, step_log);
        }

        // prevent overflow of n.
        while(n > N)
        {
            n -= N;
        }
    }

    const Value_type& get() const
    {
        return r.get();
    }
        
private:
    Recorder r;
    int N, n;

};


/**
 * "Wraps" and existing recorder, making it output the recorded value to the given
 * stream.
 *
 * @tparam Recorder A type implemeting the ModelRecorder concept.
 *
 * @warning The wrapped recorder is stored by reference, so the caller must ensure
 * it isn't freed while Modulo_recorder is in use.
 */
template <class Recorder>
class Ostream_recorder
{
public:
    BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));;

    typedef typename Recorder::Model_type Model_type;
    typedef typename Recorder::Value_type Value_type;
    typedef boost::function2<std::ostream&, std::ostream&, const Value_type&> Write_function;

//    /**
//     * @param recorder  The recorder to wrap.  This will be stored be value,
//     *                  so the caller is responsible for making sure the
//     *                  recorder remains in scope
//     * @param os        ostream to output.
//     */
//    Ostream_recorder(const Recorder& recorder, std::ostream& os, const std::string& separator = "\n") :
//        r(recorder),
//        m_os(os),
//        m_sep(separator),
//        write_func_(static_cast<std::ostream&(*)(std::ostream&, const Value_type&)>(operator<<))
//    {
//        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));
//
//        if(!m_os)
//        {
//            KJB_THROW_2(kjb::IO_error, "Error with output stream");
//        }
//    }

    /**
     * @param recorder  The recorder to wrap.  This will be stored be value,
     *                  so the caller is responsible for making sure the
     *                  recorder remains in scope
     * @param os        ostream to output.
     * @param write_func The write function to call (alternative to operator>>, with same signature)
     */
    Ostream_recorder(
            const Recorder& recorder,
            std::ostream& os,
            const std::string& separator = "\n",
            const Write_function& write_func = Ostream_recorder::default_write) :
        r(recorder),
        m_os(os),
        m_sep(separator),
        write_func_(write_func)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));

        if(!m_os)
        {
            KJB_THROW_2(kjb::IO_error, "Error with output stream");
        }
    }

    virtual void operator()(const Model_type& m, const Step_log<Model_type>& step_log)
    {
        r(m, step_log);
        write_func_(m_os, r.get());
        m_os << m_sep;

        m_os << std::flush;

        if(!m_os)
        {
            KJB_THROW_2(kjb::IO_error, "Error with output stream");
        }
    }

    const Value_type& get() const
    {
        return r.get();
    }

    static std::ostream& default_write(std::ostream& ost, const Value_type& v)
    {
        return ost << v;
    }
        
private:
    Recorder r;
    std::ostream& m_os;
    std::string m_sep;
    Write_function write_func_;
};

template <class Recorder>
Ostream_recorder<Recorder> make_ostream_recorder
(
    const Recorder& r,
    std::ostream& ost,
    const std::string& separator = "\n"
)
{
    return Ostream_recorder<Recorder>(r, ost, separator);
}

template <class Recorder, class Write_function>
Ostream_recorder<Recorder> make_ostream_recorder
(
    const Recorder& r,
    std::ostream& ost,
    const Write_function& write_func,
    const std::string& separator = "\n"
)
{
    typedef typename Ostream_recorder<Recorder>::Write_function Function;
    Function f = write_func;
    return Ostream_recorder<Recorder>(r, ost, separator, f);
}


/**
 * "Wraps" and existing recorder, making it serialize the recorded value to a file at every iteration
 *
 * @tparam Recorder A type implemeting the ModelRecorder concept.
 *
 * @warning Because the wrapped recorder is stored by reference, the caller must ensure
 * it isn't freed while Serialization_recorder is in use.
 */
template <class Recorder_type>
class Serialize_recorder
{
public:
typedef typename Recorder_type::Model_type Model_type;
typedef typename Recorder_type::Value_type Value_type;
    
    /**
     * @param recorder The ModelRecorder to wrap.  This object's get function must be serializable.
     * @param fname_fmt A printf-style format string that includes one integer field.  E.g. "output_%0d" will result in files named "output_00, output_01, ...".
     * @param iterations The number of iterations per file.  If iterations is N, then the N-th iteration will trigger an output, the 2N-th iteration, etc..  Setting iterations to zero disables writing.
     */
    Serialize_recorder(const Recorder_type& r, const std::string& fname_fmt, size_t iterations) :
        recorder_(r),
        fname_fmt_(fname_fmt),
        period_(iterations),
        it_counter_(0),
        file_counter_(0),
        tmp(fname_fmt)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder_type>));
        BOOST_CONCEPT_ASSERT((kjb::SerializableConcept<Value_type>));

        try{
            std::string fname = str(boost::format(fname_fmt_) % file_counter_);
        }
        catch(...)
        {
            KJB_THROW_2(kjb::Illegal_argument, "Invalid filename format.  Must be a printf-formatted string with one integer field.");
        }

    }
    
    /**
     * Typical record callback.  Calls wrapped recorder's callback and increments a counter.
     * If the counter is high enough, output is written to a file and the counter is reset.
     */
    void operator()(const Model_type& model, const Step_log<Model_type>& log)
    {
        // record as usual
        recorder_(model, log);

        if(period_ == 0) return;

        it_counter_++;

        // if enough iterations have passed, write to file
        if(it_counter_ == period_)
        {
            it_counter_ = 0;
            write();
        }

    }

    const Value_type& get() const
    {
        recorder_.get();
    }

private:
    void write()
    {
        const Value_type& value = recorder_.get();

// KJB_DEPENDENCY
        // for some reason, reusing some persistent boost:format object here fails. so we reconstruct it each time.
        std::string fname = str(boost::format(fname_fmt_) % file_counter_);
        kjb::save(value, fname);

        file_counter_++;
    }

    Recorder_type recorder_;
    std::string fname_fmt_;
    size_t period_;

    // iteration counter
    size_t it_counter_;
    size_t file_counter_;

    std::string tmp;
};

template <class Recorder>
Serialize_recorder<Recorder> make_serialization_recorder(const Recorder& r, std::string fname_fmt, size_t iterations = 1)
{
    return Serialize_recorder<Recorder>(r, fname_fmt, iterations);
}




/**
 * @class Null_recorder
 *
 * "No-op" recorder.
 */
template <class Model>
class Null_recorder
{
public:
    typedef Null_value Value_type;
    typedef Model Model_type;
    void operator()(const Model_type&, const Step_log<Model>&)
    {}

    const Value_type& get() const {static Value_type out; return out;}
};

/**
 * Record using a user-specified callback.
 */
template <class Model>
class Callback_recorder : public Null_recorder<Model>
{
    typedef Null_recorder<Model> Base;
public:
    typedef typename Base::Value_type Value_type;
    typedef typename Base::Model_type Model_type;
    template <class Function>
    Callback_recorder(Function f) :
        f_(f)
    { }

    void operator()(const Model_type& model, const Step_log<Model>& step_log)
    {
        return f_(model, step_log);
    }
private:
    boost::function2<void, const Model_type&, const Step_log<Model>&> f_;
};

/**
 * A wrapping recorder that connects a recorder to another class that has
 * an updatable view of the recorded item.
 *
 * @tparam Model    A type implemeting the BaseModel concept
 * @tparam Recorder A type implementing the ModelRecorder concept.
 * @tparam Updater  A type implementing the Updatable concept,
 *                  whose ::update_type is same as Recorder's ::value_type
 */
template <class Recorder, class Updater>
class Viewing_recorder
{
public:
    BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));
    BOOST_CONCEPT_ASSERT((Updatable<Updater>));

    typedef typename Recorder::Value_type Value_type;
    typedef typename Recorder::Model_type Model_type;
    typedef typename Updater::Update_type Update_type;

    // Recorder's output must match Updater's input.
    //BOOST_CONCEPT_ASSERT((boost::Convertible<Value_type, Update_type>));

public:
    Viewing_recorder(const Recorder& r, Updater& u) :
        m_recorder(r),
        m_updater(u)
    {}

    void operator()(const Model_type& model, const Step_log<Model_type>& step_log)
    {
        // pass through to recorder
        m_recorder(model, step_log);

        // update viewer
        m_updater.update(m_recorder.get());
    }

    const Value_type& get() const
    {
        return m_recorder.get();
    }

private:
    Recorder m_recorder;
    Updater& m_updater;
};

#endif

