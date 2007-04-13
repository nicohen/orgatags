/*
 * GrowArray.h
 * 
 * Templatized array of arbitrary size.  Lookups in constant time.
 * Appends in constant amortized time, though an individual operation
 * may take up to O(n) time.
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#ifndef GROWARRAY_H
#define GROWARRAY_H

template <class T>
class GrowArray
{
private:
    int maxSize;
    int numElems;
    T*  array;

public:
    GrowArray(int initialSize = 16)
    {
        if (initialSize < 1) {
            initialSize = 1;
        }

        maxSize = initialSize;
        numElems = 0;
        array = new T[maxSize];
    }

    GrowArray(const GrowArray& other)
    {
        maxSize = other.maxSize;
        numElems = other.numElems;
        array = new T[maxSize];

        for (int i = 0; i < numElems; i++) {
            array[i] = other.array[i];
        }
    }

    ~GrowArray()
    {
        delete [] array;
    }

    // Add one more element to the array, growing it if necessary.
    void append(const T& newElem)
    {
        array[numElems++] = newElem;
        if (numElems == maxSize) {
            grow(maxSize * 2);
        }
    }

    void deleteLast()
    {
        if (numElems > 0) {
            numElems--;
        }
    }

    void sort( int(*cmpfun)(const void *a, const void *b)) {
        qsort(array, numElems, sizeof(T), cmpfun);
    }

    // Get the used or total space in the array.
    int size() const                    { return numElems; }
    int length() const                  { return numElems; }
    //int space() const                   { return maxSize; }

    // Access an element in the array (const or not)
    T& operator[](int i)                { return array[i]; }
    const T& operator[](int i) const    { return array[i]; }

    // Get the entire underlying array (const or not)
    T* getArray()                       { return array; }
    const T* getArray() const           { return array; }

    // Assignment operator
    GrowArray<T>& operator=(const GrowArray<T>& other)
    {
        delete [] array;
        maxSize = other.maxSize;
        numElems = other.numElems;
        array = new T[maxSize];
        
        for (int i = 0; i < numElems; i++) {
            array[i] = other.array[i];
        }

        return *this;
    }

private:
    void grow(int newSize)
    {
        T* oldArray = array;
        array = new T[newSize];
        for (int i = 0; i < numElems; i++) {
            array[i] = oldArray[i];
        }
        maxSize = newSize;
        delete [] oldArray;
    }
};

#endif // ndef GROWARRAY_H
