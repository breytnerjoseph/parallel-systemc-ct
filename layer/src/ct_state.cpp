#include "../include/ct_state.h"


#ifdef FIXED_STATE 

std::ostream& operator<<( std::ostream &out , const sct_core::my_vector &p )
{
    for (auto it: p) {
        out << it << " ";   
    }
    return out;
}


sct_core::my_vector::my_vector( const sct_core::my_vector &p )
    : m_v(p.m_v)
{
}

sct_core::my_vector::my_vector( const vector &p )
    : m_v(p)
{
}


sct_core::my_vector::my_vector( const size_t N ) 
{
    for (int i = 0; i < INITIAL_SIZE; i++) {
        m_v(i) = 0.0;
    }
    // std::cout << "Creating a vector of size " << N << std::endl;
}

sct_core::my_vector::my_vector()
{
    for (int i = 0; i < INITIAL_SIZE; i++) {
        m_v(i) = 0.0;
    }
}

const double &  sct_core::my_vector::operator[]( const size_t n ) const {
    return m_v(n);
}

double &  sct_core::my_vector::operator[]( const size_t n ) { 
    if( (n+1) > this->size() ){
    
    //     resize(n+1);
        // std::cout << "Trying to get el " << n << " but size " << this->size() << std::endl;  
        return m_v(0);
    }

    return m_v(n); 
}

sct_core::my_vector::iterator  sct_core::my_vector::begin() { 
    return m_v.begin();
}

sct_core::my_vector::const_iterator  sct_core::my_vector::begin() const {
    return m_v.begin(); 
}

sct_core::my_vector::iterator  sct_core::my_vector::end() { 
    return m_v.end(); 
}

sct_core::my_vector::const_iterator sct_core::my_vector::end() const { 
    return m_v.end(); 
}

size_t  sct_core::my_vector::size() const { 
    return m_v.size(); 
}

void  sct_core::my_vector::resize( const size_t n ) {
    // m_v.resize(n);   

    // std::cout << "Trying to resize from" << m_v.size() <<" to " << n << std::endl;
}

sct_core::my_vector&  sct_core::my_vector::operator+=( const my_vector &p ) {

    int _size = p.m_v.size();
    if(_size == 0){
        return *this;
    }

    if (_size != m_v.size() ) {
        std::cout << "Adding vector of size " << _size << " to " << m_v.size() << std::endl;
      //  resize(_size);
    }

    m_v += p.m_v; 

    return *this;
}

sct_core::my_vector&  sct_core::my_vector::operator*=( double factor )
{  
    m_v *= factor;
    
    return *this;
}


sct_core::my_vector sct_core::operator/( const sct_core::my_vector &p1 , const sct_core::my_vector &p2 )
{

    size_t _size = p1.m_v.size();
    sct_core::my_vector result(_size);

    for (size_t i = 0; i < _size; i++){
        result.m_v(i) = p1.m_v(i) / p2.m_v(i);
    }

    return result;
}

sct_core::my_vector sct_core::operator*( const sct_core::my_vector &p1 , double factor )
{
    sct_core::my_vector result(p1.m_v * factor);

    return result;
}

sct_core::my_vector sct_core::operator*(double factor, const my_vector &p1) {
    return p1 * factor;
}

sct_core::my_vector sct_core::operator+(const my_vector &p1 , const my_vector &p2) {
    sct_core::my_vector result(p1);

    result += p2;

    return result;
}

sct_core::my_vector sct_core::operator-(const my_vector &p1 , const my_vector &p2) {
    sct_core::my_vector result(p1);

    result += -1*p2;

    return result;
}

// sct_core::my_vector sct_core::operator+( const sct_core::my_vector &p1 , const sct_core::my_vector &p2)
// {
//     sct_core::my_vector result(p1);

//     result.m_v += p2.m_v;

//     return result;
// }


sct_core::my_vector sct_core::abs( const sct_core::my_vector &p )
{
    size_t _size = p.m_v.size();
    sct_core::my_vector result(_size);

    for (size_t i = 0; i < _size; i++){
        result.m_v(i) = std::fabs(p.m_v(i));
    }

    return result;
}









#else 



///////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream &out , const sct_core::my_vector &p )
{
    for (auto it: p) {
        out << it << " ";   
    }
    return out;
}


sct_core::my_vector::my_vector( const sct_core::my_vector &p )
    : m_v(p.m_v)
{
}

sct_core::my_vector::my_vector( const vector &p )
    : m_v(p)
{
}


sct_core::my_vector::my_vector( const size_t N )
    : m_v(N)
{
}

sct_core::my_vector::my_vector()
    : m_v(INITIAL_SIZE)
{
}


sct_core::my_vector::iterator  sct_core::my_vector::begin() { 
    return m_v.begin();
}

sct_core::my_vector::const_iterator  sct_core::my_vector::begin() const {
    return m_v.begin(); 
}

sct_core::my_vector::iterator  sct_core::my_vector::end() { 
    return m_v.end(); 
}

sct_core::my_vector::const_iterator sct_core::my_vector::end() const { 
    return m_v.end(); 
}

size_t  sct_core::my_vector::size() const { 
    return m_v.size(); 
}

void  sct_core::my_vector::resize( const size_t n ) {
    m_v.resize(n);
}

sct_core::my_vector&  sct_core::my_vector::operator+=( const my_vector &p ) {

    int _size = p.m_v.size();
    if(_size == 0){
        return *this;
    }

    if (_size != m_v.size() ) { 
       resize(_size);
    }

    m_v += p.m_v; 

    return *this;
}

sct_core::my_vector&  sct_core::my_vector::operator*=( double factor )
{  
    m_v *= factor;
    
    return *this;
}

sct_core::my_vector sct_core::operator*( const sct_core::my_vector &p1 , double factor )
{
    sct_core::my_vector result(p1.m_v * factor);

    return result;
}

sct_core::my_vector sct_core::operator*(double factor, const my_vector &p1) {
    return sct_core::my_vector(p1.m_v * factor);
}


sct_core::my_vector sct_core::operator/( const sct_core::my_vector &p1 , const sct_core::my_vector &p2 )
{

    size_t _size = p1.m_v.size();
    sct_core::my_vector result(_size);

    for (size_t i = 0; i < _size; i++){
        result.m_v(i) = p1.m_v(i) / p2.m_v(i);
    }

    return result;
}



sct_core::my_vector sct_core::operator+(const my_vector &p1 , const my_vector &p2) {
    sct_core::my_vector result(p1);

    result += p2;

    return result;
}

sct_core::my_vector sct_core::operator-(const my_vector &p1 , const my_vector &p2) {
    sct_core::my_vector result(p1);

    result += -1*p2;

    return result;
}

// sct_core::my_vector sct_core::operator+( const sct_core::my_vector &p1 , const sct_core::my_vector &p2)
// {
//     sct_core::my_vector result(p1);

//     result.m_v += p2.m_v;

//     return result;
// }


sct_core::my_vector sct_core::abs( const sct_core::my_vector &p )
{
    size_t _size = p.m_v.size();
    sct_core::my_vector result(_size);

    for (size_t i = 0; i < _size; i++){
        result.m_v(i) = std::fabs(p.m_v(i));
    }

    return result;
}

#endif