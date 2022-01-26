#ifndef CT_STATE_H
#define CT_STATE_H


#include <boost/operators.hpp>
#include <boost/numeric/odeint.hpp>

	
#include <Eigen/Core>


// CT class based on examples from 
// https://www.boost.org/doc/libs/1_62_0/libs/numeric/odeint/doc/html/boost_numeric_odeint/odeint_in_detail/state_types__algebras_and_operations.html

#define FIXED_STATE

#ifdef FIXED_STATE

namespace sct_core{
    
    class my_vector // : 
       // boost::additive1< my_vector ,
       // boost::additive2< my_vector , double ,
       // boost::multiplicative2< my_vector , double > > >
    {
        // typedef std::vector< double > vector;
        // typedef boost::numeric::ublas::vector< double > vector;
        //  typedef Eigen::VectorXd  vector;
        
        #define INITIAL_SIZE 2
        
        typedef Eigen::Matrix<double, INITIAL_SIZE, 1> vector;
        // typedef boost::array<double,INITIAL_SIZE> vector;

    public:
        typedef vector::iterator iterator;
        typedef vector::const_iterator const_iterator;

    public:
        my_vector( const size_t N );

        my_vector();
        my_vector( const sct_core::my_vector &p );
        my_vector( const vector &p );

        const double & operator[]( const size_t n ) const;

        double & operator[]( const size_t n );

        iterator begin();
        const_iterator begin() const;

        iterator end();
        
        const_iterator end() const;

        size_t size() const;

        void resize( const size_t n );

        my_vector& operator+=( const my_vector &p );

        my_vector& operator*=( double factor );

        const vector &get_core() const {
            return m_v;
        }

        vector &get_core(){
            return m_v;
        }

        friend my_vector operator*( const my_vector &p1 , double);
        friend my_vector operator*( double, const my_vector &p1);
        friend my_vector operator+( const my_vector &p1 , const my_vector &p2);
        friend my_vector operator-( const my_vector &p1 , const my_vector &p2);


        friend my_vector operator/( const my_vector &p1 , const my_vector &p2 );
        friend my_vector abs( const my_vector &p );

        friend class boost::numeric::odeint::vector_space_norm_inf<sct_core::my_vector >;

    private:
        vector m_v;

    };

    my_vector operator/( const my_vector &p1 , const my_vector &p2 );

    my_vector operator*(const my_vector &p1 , double);
    my_vector operator*(double, const my_vector &p1);
    my_vector operator+(const my_vector &p1 , const my_vector &p2);
    my_vector operator-(const my_vector &p1 , const my_vector &p2);



    my_vector abs( const my_vector &p );

    typedef my_vector ct_state;
}

std::ostream& operator<<( std::ostream &out , const sct_core::my_vector &p );

 namespace boost { namespace numeric { namespace odeint {

    // template<>
    // struct is_resizeable< sct_core::my_vector >
    // {
    //     typedef boost::true_type type;
    //     static const bool value = type::value;
    // };

    } } }


    namespace boost { namespace numeric { namespace odeint {

    template<>
    struct vector_space_norm_inf< sct_core::my_vector >
    {
        static double abs_compare(double a, double b)
        {
            double _a = std::fabs(a), _b = std::fabs(b);
            if (_a < _b){
                return _b;
            }
            return _a;
        }
        
        typedef double result_type;
        double operator()( const sct_core::my_vector &v ) const
        {
            double max = *v.m_v.begin();
            double comp;
            for( auto el: v.m_v ){
                comp = std::fabs(el); 
                if( comp > max){
                    max = comp;
                }
            }
            return max;
        }
    };

    } } }
    




#else 



namespace sct_core{
    
    class my_vector // : 
       // boost::additive1< my_vector ,
       // boost::additive2< my_vector , double ,
       // boost::multiplicative2< my_vector , double > > >
    {
        // typedef std::vector< double > vector;
        // typedef boost::numeric::ublas::vector< double > vector;
         typedef Eigen::VectorXd  vector;
        
        #define INITIAL_SIZE 3
        
        // typedef Eigen::Matrix<double, INITIAL_SIZE, 1> vector;
        // typedef boost::array<double,INITIAL_SIZE> vector;

    public:
        typedef vector::iterator iterator;
        typedef vector::const_iterator const_iterator;

    public:
        my_vector( const size_t N );

        my_vector();
        my_vector( const sct_core::my_vector &p );
        my_vector( const vector &p );

        inline const double &  operator[]( const size_t n ) const {
            return m_v(n);
        }

        inline double &  operator[]( const size_t n ) { 
            // static double ret = 0;
            // if( (n+1) > this->size() ){
            //     resize(n+1);
            //     std::cout << "Error accessing " << n << " bu size is " << size() << std::endl;
            //     exit(EXIT_FAILURE);
            //     return ret;
            // }

            return m_v(n); 
        }

        iterator begin();
        const_iterator begin() const;

        iterator end();
        
        const_iterator end() const;

        size_t size() const;

        void resize( const size_t n );

        my_vector& operator+=( const my_vector &p );

        my_vector& operator*=( double factor );

        const vector &get_core() const {
            return m_v;
        }

        vector &get_core(){
            return m_v;
        }

        friend my_vector operator*( const my_vector &p1 , double);
        friend my_vector operator*( double, const my_vector &p1);
        friend my_vector operator+( const my_vector &p1 , const my_vector &p2);
        friend my_vector operator-( const my_vector &p1 , const my_vector &p2);


        friend my_vector operator/( const my_vector &p1 , const my_vector &p2 );
        friend my_vector abs( const my_vector &p );

        friend class boost::numeric::odeint::vector_space_norm_inf<sct_core::my_vector >;

    private:
        vector m_v;

    };

    my_vector operator/( const my_vector &p1 , const my_vector &p2 );

    my_vector operator*(const my_vector &p1 , double);
    my_vector operator*(double, const my_vector &p1);
    my_vector operator+(const my_vector &p1 , const my_vector &p2);
    my_vector operator-(const my_vector &p1 , const my_vector &p2);



    my_vector abs( const my_vector &p );

    typedef my_vector ct_state;
}



std::ostream& operator<<( std::ostream &out , const sct_core::my_vector &p );

 namespace boost { namespace numeric { namespace odeint {

    template<>
    struct is_resizeable< sct_core::my_vector >
    {
        typedef boost::true_type type;
        static const bool value = type::value;
    };

    } } }


    namespace boost { namespace numeric { namespace odeint {

    template<>
    struct vector_space_norm_inf< sct_core::my_vector >
    {
        static double abs_compare(double a, double b)
        {
            double _a = std::fabs(a), _b = std::fabs(b);
            if (_a < _b){
                return _b;
            }
            return _a;
        }
        
        typedef double result_type;
        double operator()( const sct_core::my_vector &v ) const
        {
            double max = *v.m_v.begin();
            double comp;
            for( auto el: v.m_v ){
                comp = std::fabs(el); 
                if( comp > max){
                    max = comp;
                }
            }
            return max;
        }
    };

    } } }


#endif 

#endif

