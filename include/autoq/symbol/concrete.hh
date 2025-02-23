#ifndef _AUTOQ_CONCRETE_HH_
#define _AUTOQ_CONCRETE_HH_

#include <vector>
#include <autoq/util/convert.hh>
#include <autoq/complex/complex.hh>
#include <boost/multiprecision/cpp_int.hpp>

namespace AUTOQ
{
	namespace Symbol
	{
        struct Concrete;
	}
}

// Concrete symbol
struct AUTOQ::Symbol::Concrete {
private:
    bool internal;
public:
    Complex::Complex complex;

    // Notice that if we do not use is_convertible_v, type int will not be accepted in this case.
    template <typename T, typename = std::enable_if_t<std::is_convertible<T, boost::multiprecision::cpp_int>::value>>
        Concrete(T qubit) : internal(true), complex(qubit) {}
    Concrete(const Complex::Complex &c) : internal(false), complex(c) {}
    Concrete() : internal(), complex() {} // prevent the compiler from complaining about the lack of default constructor
    bool is_internal() const { return internal; }
    bool is_leaf() const { return !internal; }
    boost::multiprecision::cpp_int qubit() const {
        assert(internal);
        // assert(complex.real().denominator() == 1);
        return complex.toInt(); //.numerator();
    }
    void back_to_zero() { complex = Complex::Complex::Zero(); }
    friend std::ostream& operator<<(std::ostream& os, const Concrete& obj) {
        if (obj.internal)
            os << "[" + obj.qubit().str() + "]";
        else
            os << obj.complex;
        return os;
    }
    Concrete operator+(const Concrete &o) const { return Concrete(complex.operator+(o.complex)); }
    Concrete operator-(const Concrete &o) const { return Concrete(complex.operator-(o.complex)); }
    Concrete operator*(const Concrete &o) const { return Concrete(complex.operator*(o.complex)); }
    bool operator==(const Concrete &o) const { return internal == o.internal && complex == o.complex; }
    bool operator<(const Concrete &o) const {
        if (internal && !o.internal) return true;
        if (o.internal && !internal) return false;
        return complex < o.complex;
    }
    void omega_multiplication(int rotation=1) {
        if (rotation > 0) complex.counterclockwise(boost::rational<boost::multiprecision::cpp_int>(rotation, 8));
        if (rotation < 0) complex.clockwise(boost::rational<boost::multiprecision::cpp_int>(rotation, 8));
    }
    void fraction_simplification() { complex.fraction_simplification(); }
    void divide_by_the_square_root_of_two() { complex.divide_by_the_square_root_of_two(); }
    void negate() { complex = complex * (-1); }
    void degree45cw() { complex.clockwise(boost::rational<boost::multiprecision::cpp_int>(1, 8)); }
    void degree90cw() { complex.clockwise(boost::rational<boost::multiprecision::cpp_int>(1, 4)); }
};

namespace boost {
    template <> struct hash<AUTOQ::Symbol::Concrete> {
        size_t operator()(const AUTOQ::Symbol::Concrete& obj) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, obj.is_internal());
            boost::hash_combine(seed, obj.complex);
            return seed;
        }
    };
}

#endif