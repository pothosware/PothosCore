///
/// \file Object/Object.hpp
///
/// Object is intended to facilitate API polymorphism similar to java.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/Templates.hpp>
#include <typeinfo>
#include <string>

namespace Pothos {

//messy forward declares
namespace Detail {
struct ObjectContainer;
} //namespace Detail

/*!
 * The NullObject class: Object().type() == typeid(NullObject).
 * This type is used by the Object.type() to represent a null Object.
 * A null Object can be constructed using an instance of NullObject.
 */
class POTHOS_API NullObject
{
public:
    NullObject(void);
    ~NullObject(void);
};

/*!
 * Object is a general type capable of storing arbitrary data types.
 * When an Object instance is copied, the internal data is not copied.
 * The internal data is only deleted when all Object copies are gone.
 *
 * - Making a new object: int MyValue = 42; Object foo(myValue);
 * - Extracting an object (reference): const int &val = foo.extract<int>();
 * - Converting an object (safe): const int long = foo.convert<long>();
 * - Check if an object is null: myObj.null() or myObj == Object();
 * - Check if an object is a type: myObj.type() == typeid(someType);
 */
class POTHOS_API Object
{
public:
    /*!
     * Create a null Object.
     */
    Object(void);

    /*!
     * Create an object where type can be explicitly specified.
     * The make method allows users to pass an explicit type,
     * which is not possible using the templated constructor.
     * Also, the make method can create an Object containing another object;
     * which is not possible with constructors because the copy constructor.
     * \param value a value of arbitrary type to put in an Object
     * \return a new Object containing a copy of the specified value
     */
    template <typename ValueType>
    static Object make(ValueType &&value);

    /*!
     * Create an object with emplacement construction.
     * The make method allows users to pass an explicit type,
     * and an arbitrary number of constructor arguments
     * which will be forwarded to the type T constructor.
     * \tparam T the internal type held by the new Object
     * \param args the constructor arguments for type T
     * \return a new Object of type T constructed from args
     */
    template <typename T, typename... Args>
    static Object make(Args&&... args);

    /*!
     * Copy constructor for Object -- does not copy the internal data.
     * Both obj and the resulting Object will point to the same data.
     * \param obj another Object
     */
    Object(const Object &obj);

    /*!
     * Move constructor for Object.
     * The contents of obj will be moved to the new Object.
     * \param obj another Object
     */
    Object(Object &&obj) noexcept;

    /*!
     * Create a new Object from an an arbitrary value.
     * \param value the data to store internally
     */
    template <typename ValueType, typename = Pothos::Util::disable_if_same<Object, ValueType>>
    explicit Object(ValueType &&value);

    /*!
     * Destructor for Object.
     * If this Object holds the only copy of the internal data,
     * then the internal data will be deleted in the destructor.
     */
    ~Object(void);

    /*!
     * Object copy assignment.
     * This Object will store a copy of the internal data in rhs.
     * The old internal data will be released, and deleted if last.
     * \param rhs another Object
     */
    Object &operator=(const Object &rhs);

    /*!
     * Object move assignment.
     * The contents of rhs will be moved to this Object.
     * The old internal data will be released, and deleted if last.
     * \param rhs another Object
     */
    Object &operator=(Object &&rhs);

    /*!
     * Does the object hold a value?
     * \return true if the container is non-empty
     */
    explicit operator bool(void) const;

    /*!
     * Is the Object unique?
     * \return true if this is the only reference
     */
    bool unique(void) const;

    /*!
     * Get the type of the internal data.
     * The call returns typeid(NullObject) if the Object is null.
     */
    const std::type_info &type(void) const;

    /*!
     * Get a const reference to the internal data.
     * The requested cast type must exactly match the type().
     * \throws ObjectConvertError if object type != ValueType
     * \return a const reference to the internal data
     */
    template <typename ValueType>
    const ValueType &extract(void) const;

    /*!
     * Get a non-const reference to the internal data.
     * The requested cast type must exactly match the type().
     * The Object must be unique and have only one reference.
     *
     * Note: The Object container class is usually immutable
     * unless the caller has sole ownership of the object.
     * Typical use cases for mutability might include
     * object initialization, object de-serialization,
     * and rvalue m when object is a return value.
     *
     * \throws ObjectConvertError if object type != ValueType
     * \throws ObjectConvertError if multiple reference counts
     * \return a non-const reference the internal data
     */
    template <typename ValueType>
    ValueType &ref(void);

    /*!
     * Convert performs a safe conversion that respects the value
     * when an exact type conversion is not required or wanted.
     * For example, the object may contain an int, we want a long.
     * \throws ObjectConvertError if object cannot be safe casted
     * \return a value that represents the internal data
     */
    template <typename ValueType>
    ValueType convert(void) const;

    /*!
     * Templated conversion operator to assign Object to a target type.
     * \throws ObjectConvertError if object cannot be safe casted
     */
    template <typename ValueType>
    operator ValueType(void) const;

    /*!
     * Convert to a new Object that will be of the type specified.
     * \throws ObjectConvertError if object cannot be safe casted
     * \param type the desired type held by the output Object
     * \return a new Object that can be extracted to type
     */
    Object convert(const std::type_info &type) const;

    /*!
     * Is the conversion of this Object to specified type possible?
     * \param type the desired typeid in question
     * \return true if the conversion is possible.
     */
    bool canConvert(const std::type_info &type) const;

    /*!
     * Is the conversion from source type to destination type possible?
     * \param srcType the type of the conversion input
     * \param dstType the type of the conversion output
     * \return true if the conversion is possible.
     */
    static bool canConvert(const std::type_info &srcType, const std::type_info &dstType);

    /*!
     * Serialize the contents of the object into a stream.
     * \throws ObjectSerializeError if the type is not registered
     * \param os the output stream for the serialized data
     * \return a reference to the output stream
     */
    std::ostream &serialize(std::ostream &os) const;

    /*!
     * Deserialize the stream into the contents of this Object.
     * Only make this call on a null object.
     * \throws ObjectSerializeError if the type is not registered
     * \param is the input stream holding serialized data
     * \return a reference to the input stream
     */
    std::istream &deserialize(std::istream &is);

    /*!
     * Returns a negative integer, zero, or a positive integer as this object is
     * less than, equal to, or greater than the specified object.
     * \throws ObjectCompareError when the compare isnt possible
     * \param other the other proxy object to compare against
     * \return an int representing less than, equal to, or greater than
     */
    int compareTo(const Object &other) const;

    /*!
     * Get a hash code for the contained object.
     * The hash code should be identical for equivalent objects.
     */
    size_t hashCode(void) const;

    /*!
     * Get a printable string representation for this Object.
     * The string formatting depends upon the contained value.
     */
    std::string toString(void) const;

    /*!
     * Get a printable string representing the type held by this Object.
     */
    std::string getTypeString(void) const;

    /*!
     * Value-based equality operator.
     * When supported, the implementation uses the compareTo() method.
     * Otherwise, the hash codes are compared for checking equality.
     */
    bool equals(const Object &obj) const;

    /*!
     * Less than comparable operator.
     * When supported, the implementation uses the compareTo() method.
     * Otherwise, the hash codes are compared for sorting purposes.
     */
    bool operator<(const Object &obj) const;

    /*!
     * Greater than comparable operator.
     * When supported, the implementation uses the compareTo() method.
     * Otherwise, the hash codes are compared for sorting purposes.
     */
    bool operator>(const Object &obj) const;

    //! Private implementation details
    Detail::ObjectContainer *_impl;
};

/*!
 * The equals operators checks if two Objects represent the same memory.
 * Use myObject.compareTo(other) == 0 for an equality comparison.
 * \param lhs the left hand object of the comparison
 * \param rhs the right hand object of the comparison
 * \return true if the objects represent the same internal data
 */
inline bool operator==(const Object &lhs, const Object &rhs);

} //namespace Pothos

inline bool Pothos::operator==(const Object &lhs, const Object &rhs)
{
    return lhs._impl == rhs._impl;
}

inline Pothos::Object::Object(Object &&obj) noexcept:
    _impl(obj._impl)
{
    obj._impl = nullptr;
}
