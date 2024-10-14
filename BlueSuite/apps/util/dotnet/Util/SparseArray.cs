//------------------------------------------------------------------------------
//
// <copyright file="SparseArray.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Text;

using QTIL.HostTools.Common.EngineFrameworkClr;

// Extend System.Collections
namespace System.Collections
{
    /// <summary>
    /// Class providing a sparse array i.e. a non-contiguous array of values.
    /// </summary>
    [
    TypeConverter(typeof(ExpandableObjectConverter))
    ]
    public class SparseArray
        : IList
    {

        #region SparseArrayEnumerator

        /// <summary>
        /// 
        /// </summary>
        private class SparseArrayEnumerator
            : IEnumerator
        {
            /// <summary>
            /// 
            /// </summary>
            private readonly IDictionaryEnumerator mDictionary;

            /// <summary>
            /// Sets the enumerator to its initial position, which is before the first element in the collection.
            /// </summary>
            /// <exception cref="T:System.InvalidOperationException">
            /// The collection was modified after the enumerator was created.
            ///   </exception>
            public void Reset()
            {
                mDictionary.Reset();
            }

            /// <summary>
            /// Advances the enumerator to the next element of the collection.
            /// </summary>
            /// <returns>
            /// true if the enumerator was successfully advanced to the next element; false if the enumerator has passed the end of the collection.
            /// </returns>
            /// <exception cref="T:System.InvalidOperationException">
            /// The collection was modified after the enumerator was created.
            ///   </exception>
            public Boolean MoveNext()
            {
                return mDictionary.MoveNext();
            }

            /// <summary>
            /// Gets the current element in the collection.
            /// </summary>
            /// <returns>
            /// The current element in the collection.
            ///   </returns>
            ///   
            /// <exception cref="T:System.InvalidOperationException">
            /// The enumerator is positioned before the first element of the collection or after the last element.
            ///   </exception>
            public Object Current
            {
                get
                {
                    // Convert to 'proper' indexes
                    return new DictionaryEntry(StringToIndexes((String)mDictionary.Key), mDictionary.Value);
                }
            }

            /// <summary>
            /// Gets the index.
            /// </summary>
            public Int32[] Index
            {
                get
                {
                    return StringToIndexes((String)mDictionary.Key);
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="SparseArrayEnumerator"/> class.
            /// </summary>
            /// <param name="sparseArray">The sparse array.</param>
            public SparseArrayEnumerator(SparseArray sparseArray)
            {
                mDictionary = sparseArray.mValues.GetEnumerator();
            }
        }

        #endregion

        #region Private data

        /// <summary>
        /// Contains the values
        /// </summary>
        private readonly Dictionary<String, Object> mValues;

        /// <summary>
        /// Gets the values.
        /// </summary>
        public Dictionary<String, Object> Values
        {
            get
            {
                MessageHandler.DebugEntry();

                MessageHandler.DebugExit(mValues);

                return mValues;
            }
        }

        #endregion

        #region Properties

        /// <summary>
        /// Contains the number of dimensions
        /// </summary>
        private readonly Int32 mDimensions;

        /// <summary>
        /// Gets the rank.
        /// </summary>
        public Int32 Rank
        {
            get
            {
                MessageHandler.DebugEntry();

                Int32 retVal = mDimensions;

                MessageHandler.DebugExit(retVal);

                return retVal;
            }
        }

        #endregion

        #region Public methods

        /// <summary>
        /// Contains the lower bounds for all of the dimensions
        /// </summary>
        private Int32[] mLowerBounds;

        /// <summary>
        /// Gets the lower bound of the specified dimension.
        /// </summary>
        /// <param name="dimension">The dimension.</param>
        /// 
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="dimension"/> is greater than the defined dimensions.
        ///   </exception>
        /// 
        /// <exception cref="IndexOutOfRangeException"></exception>
        /// 
        /// <returns>The lower bound of the specified dimension.</returns>
        public Int32 GetLowerBound(Int32 dimension)
        {
            MessageHandler.DebugEntry();

            if (dimension > mDimensions)
            {
                throw new ArgumentOutOfRangeException("dimension");
            }

            Int32 retVal;
            // Handle scalar
            if (dimension == 0)
            {
                retVal = 0;
            }
            else
            {
                retVal = mLowerBounds[dimension];
            }

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Contains the upper bounds for all of the dimensions
        /// </summary>
        private Int32[] mUpperBounds;

        /// <summary>
        /// Gets the upper bound of the specified dimension.
        /// </summary>
        /// <param name="dimension">The dimension.</param>
        /// 
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="dimension"/> is greater than the defined dimensions.
        ///   </exception>
        /// 
        /// <exception cref="IndexOutOfRangeException"></exception>
        /// 
        /// <returns>The upper bound of the specified dimension.</returns>
        public Int32 GetUpperBound(Int32 dimension)
        {
            MessageHandler.DebugEntry();

            if (dimension > mDimensions)
            {
                throw new ArgumentOutOfRangeException("dimension");
            }

            Int32 retVal;
            // Handle scalar
            if (dimension == 0)
            {
                retVal = 0;
            }
            else
            {
                retVal = mUpperBounds[dimension];
            }

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Gets the span of the specified dimension.
        /// </summary>
        /// <param name="dimension">The dimension.</param>
        /// 
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="dimension"/> is greater than the defined dimensions.
        ///   </exception>
        /// 
        /// <exception cref="IndexOutOfRangeException"></exception>
        /// 
        /// <returns>The span of the specified dimension.</returns>
        public Int32 GetSpan(Int32 dimension)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugEnhancedFormat("dimension=[{0}]", dimension);

            if (dimension > mDimensions)
            {
                throw new ArgumentOutOfRangeException("dimension");
            }

            Int32 retVal;
            // Handle scalar
            if (dimension == 0)
            {
                retVal = mValues.Count;
            }
            else
            {
                retVal = mUpperBounds[dimension] - mLowerBounds[dimension];
            }

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Gets the value at the specified index(es).
        /// </summary>
        /// <param name="indexes">The indexes.</param>
        /// <returns></returns>
        public Object GetValue(Int32[] indexes)
        {
            MessageHandler.DebugEntry();

            String key = IndexesToString(indexes);
            MessageHandler.DebugEnhancedFormat("indexes=[{0}]", key);

            Object value;
            mValues.TryGetValue(key, out value);

            MessageHandler.DebugExit(value);

            return value;
        }

        /// <summary>
        /// Sets the value at the specified index(es).
        /// </summary>
        /// <param name="indexes">The indexes.</param>
        /// <param name="value">The value.</param>
        public void SetValue(Int32[] indexes, Object value)
        {
            MessageHandler.DebugEntry();

            String key = IndexesToString(indexes);
            MessageHandler.DebugEnhancedFormat("indexes=[{0}], value={1}", key, value);

            Object oldValue;
            if (mValues.TryGetValue(key, out oldValue))
            {
                mValues.Remove(key);
            }

            // Add it
            mValues.Add(key, value);

            // Maintain bounds
            for (Int32 i = 0; i < mDimensions; i++)
            {
                if (mLowerBounds[i] > indexes[i])
                {
                    mLowerBounds[i] = indexes[i];
                }
                if (mUpperBounds[i] < indexes[i])
                {
                    mUpperBounds[i] = indexes[i];
                }
            }

            MessageHandler.DebugExit();
        }

        #endregion

        #region Support methods

        const Char KEY_DELIMITER = ',';

        /// <summary>
        /// Creates a key from the indexes.
        /// </summary>
        /// <param name="indexes">The indexes.</param>
        /// <returns>A comma separated list of indexes.</returns>
        public static String IndexesToString(Int32[] indexes)
        {
            MessageHandler.DebugEntry();

            StringBuilder sb = new StringBuilder();

            // Check for scalar value
            if (indexes.Length > 0)
            {
                for (Int32 i = 0; i < indexes.Length; i++)
                {
                    sb.Append(indexes[i].ToString(CultureInfo.InvariantCulture));
                    if (i < (indexes.Length - 1))
                    {
                        sb.Append(KEY_DELIMITER);
                    }
                }
            }

            String retVal = sb.ToString();

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Converts the index key back to indexes.
        /// </summary>
        /// <param name="hash">The 'hash' as a comma separated list of indexes.</param>
        /// <returns></returns>
        public static Int32[] StringToIndexes(String hash)
        {
            MessageHandler.DebugEntry();

            MessageHandler.DebugEnhancedFormat("hash=[{0}]", hash);

            String[] subStrings = hash.Split(new String[] { KEY_DELIMITER.ToString() }, StringSplitOptions.RemoveEmptyEntries);
            Int32[] retVal = new Int32[subStrings.Length];

            // Check for scalar value
            if (subStrings.Length > 0)
            {
                for (Int32 i = 0; i < subStrings.Length; i++)
                {
                    retVal[i] = Int32.Parse(subStrings[i], CultureInfo.InvariantCulture);
                }
            }

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        #endregion

        #region IList implementation

        /// <summary>
        /// Gets a value indicating whether the <see cref="T:System.Collections.IList"/> has a fixed size.
        /// </summary>
        /// <returns>true if the <see cref="T:System.Collections.IList"/> has a fixed size; otherwise, false.
        ///   </returns>
        public Boolean IsFixedSize
        {
            get
            {
                MessageHandler.DebugEntry();

                Boolean retVal = false;

                MessageHandler.DebugExit(retVal);

                return retVal;
            }
        }

        /// <summary>
        /// Gets a value indicating whether the <see cref="T:System.Collections.IList"/> is read-only.
        /// </summary>
        /// <returns>true if the <see cref="T:System.Collections.IList"/> is read-only; otherwise, false.
        ///   </returns>
        public Boolean IsReadOnly
        {
            get
            {
                MessageHandler.DebugEntry();

                Boolean retVal = false;

                MessageHandler.DebugExit(retVal);

                return retVal;
            }
        }

        /// <summary>
        /// Gets a value indicating whether access to the <see cref="T:System.Collections.ICollection"/> is synchronized (thread safe).
        /// </summary>
        /// <returns>true if access to the <see cref="T:System.Collections.ICollection"/> is synchronized (thread safe); otherwise, false.
        ///   </returns>
        public Boolean IsSynchronized
        {
            get
            {
                MessageHandler.DebugEntry();

                Boolean retVal = false;

                MessageHandler.DebugExit(retVal);

                return retVal;
            }
        }

        /// <summary>
        /// Gets the number of elements contained in the <see cref="T:System.Collections.ICollection"/>.
        /// </summary>
        /// <returns>
        /// The number of elements contained in the <see cref="T:System.Collections.ICollection"/>.
        ///   </returns>
        public Int32 Count
        {
            get
            {
                MessageHandler.DebugEntry();

                Int32 retVal = mValues.Count;

                MessageHandler.DebugExit(retVal);

                return retVal;
            }
        }

        /// <summary>
        /// Gets an object that can be used to synchronize access to the <see cref="T:System.Collections.ICollection"/>.
        /// </summary>
        /// <returns>
        /// An object that can be used to synchronize access to the <see cref="T:System.Collections.ICollection"/>.
        ///   </returns>
        public Object SyncRoot
        {
            get
            {
                return null;
            }
        }

        /// <summary>
        /// Copies the elements of the <see cref="T:System.Collections.ICollection"/> to an <see cref="T:System.Array"/>, starting at a particular <see cref="T:System.Array"/> index.
        /// </summary>
        /// <param name="array">The one-dimensional <see cref="T:System.Array"/> that is the destination of the elements copied from <see cref="T:System.Collections.ICollection"/>. The <see cref="T:System.Array"/> must have zero-based indexing.</param>
        /// <param name="index">The zero-based index in <paramref name="array"/> at which copying begins.</param>
        /// <exception cref="T:System.ArgumentNullException">
        ///   <paramref name="array"/> is null.
        ///   </exception>
        ///   
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="index"/> is less than zero.
        ///   </exception>
        ///   
        /// <exception cref="T:System.ArgumentException">
        ///   <paramref name="array"/> is multidimensional.
        /// -or-
        ///   <paramref name="index"/> is equal to or greater than the length of <paramref name="array"/>.
        /// -or-
        /// The number of elements in the source <see cref="T:System.Collections.ICollection"/> is greater than the available space from <paramref name="index"/> to the end of the destination <paramref name="array"/>.
        ///   </exception>
        ///   
        /// <exception cref="T:System.ArgumentException">
        /// The type of the source <see cref="T:System.Collections.ICollection"/> cannot be cast automatically to the type of the destination <paramref name="array"/>.
        ///   </exception>
        public void CopyTo(Array array, Int32 index)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugExit();
            throw new NotImplementedException();
        }

        /// <summary>
        /// Returns an enumerator that iterates through a collection.
        /// </summary>
        /// <returns>
        /// An <see cref="T:System.Collections.IEnumerator"/> object that can be used to iterate through the collection.
        /// </returns>
        public IEnumerator GetEnumerator()
        {
            MessageHandler.DebugEntry();

            IEnumerator retval = new SparseArrayEnumerator(this);

            MessageHandler.DebugExit();

            return retval;
        }

        /// <summary>
        /// Removes the <see cref="T:System.Collections.IList"/> item at the specified index.
        /// </summary>
        /// <param name="index">The zero-based index of the item to remove.</param>
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="index"/> is not a valid index in the <see cref="T:System.Collections.IList"/>.
        ///   </exception>
        ///   
        /// <exception cref="T:System.NotSupportedException">
        /// The <see cref="T:System.Collections.IList"/> is read-only.
        /// -or-
        /// The <see cref="T:System.Collections.IList"/> has a fixed size.
        ///   </exception>
        public void RemoveAt(Int32 index)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugEnhancedFormat("index={0}", index);

            MessageHandler.DebugExit();
            throw new NotImplementedException();
        }

        /// <summary>
        /// Inserts an item to the <see cref="T:System.Collections.IList"/> at the specified index.
        /// </summary>
        /// <param name="index">The zero-based index at which <paramref name="value"/> should be inserted.</param>
        /// <param name="value">The <see cref="T:System.Object"/> to insert into the <see cref="T:System.Collections.IList"/>.</param>
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="index"/> is not a valid index in the <see cref="T:System.Collections.IList"/>.
        ///   </exception>
        ///   
        /// <exception cref="T:System.NotSupportedException">
        /// The <see cref="T:System.Collections.IList"/> is read-only.
        /// -or-
        /// The <see cref="T:System.Collections.IList"/> has a fixed size.
        ///   </exception>
        ///   
        /// <exception cref="T:System.NullReferenceException">
        ///   <paramref name="value"/> is null reference in the <see cref="T:System.Collections.IList"/>.
        ///   </exception>
        public void Insert(Int32 index, Object value)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugEnhancedFormat("index={0}, value={1}", index, value);

            MessageHandler.DebugExit();
            throw new NotImplementedException();
        }

        /// <summary>
        /// Removes the first occurrence of a specific object from the <see cref="T:System.Collections.IList"/>.
        /// </summary>
        /// <param name="value">The <see cref="T:System.Object"/> to remove from the <see cref="T:System.Collections.IList"/>.</param>
        /// <exception cref="T:System.NotSupportedException">
        /// The <see cref="T:System.Collections.IList"/> is read-only.
        /// -or-
        /// The <see cref="T:System.Collections.IList"/> has a fixed size.
        ///   </exception>
        public void Remove(Object value)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugEnhancedFormat("value={0}", value);

            MessageHandler.DebugExit();
            throw new NotImplementedException();
        }

        /// <summary>
        /// Determines whether the <see cref="T:System.Collections.IList"/> contains a specific value.
        /// </summary>
        /// <param name="value">The <see cref="T:System.Object"/> to locate in the <see cref="T:System.Collections.IList"/>.</param>
        /// <returns>
        /// true if the <see cref="T:System.Object"/> is found in the <see cref="T:System.Collections.IList"/>; otherwise, false.
        /// </returns>
        public Boolean Contains(Object value)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugEnhancedFormat("value={0}", value);

            Boolean retVal = mValues.ContainsValue(value);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Removes all items from the <see cref="T:System.Collections.IList"/>.
        /// </summary>
        /// <exception cref="T:System.NotSupportedException">
        /// The <see cref="T:System.Collections.IList"/> is read-only.
        ///   </exception>
        public void Clear()
        {
            MessageHandler.DebugEntry();

            mValues.Clear();

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Determines the index of a specific item in the <see cref="T:System.Collections.IList"/>.
        /// </summary>
        /// <param name="value">The <see cref="T:System.Object"/> to locate in the <see cref="T:System.Collections.IList"/>.</param>
        /// <returns>
        /// The index of <paramref name="value"/> if found in the list; otherwise, -1.
        /// </returns>
        public Int32 IndexOf(Object value)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugEnhancedFormat("value={0}", value);

            if (mDimensions != 1)
            {
                MessageHandler.DebugExit();
                throw new RankException();
            }

            MessageHandler.DebugExit(0);

            return 0;
        }

        /// <summary>
        /// Adds an item to the <see cref="T:System.Collections.IList"/>.
        /// </summary>
        /// <param name="value">The <see cref="T:System.Object"/> to add to the <see cref="T:System.Collections.IList"/>.</param>
        /// <returns>
        /// The position into which the new element was inserted.
        /// </returns>
        /// <exception cref="T:System.NotSupportedException">
        /// The <see cref="T:System.Collections.IList"/> is read-only.
        /// -or-
        /// The <see cref="T:System.Collections.IList"/> has a fixed size.
        ///   </exception>
        public Int32 Add(Object value)
        {
            MessageHandler.DebugEntry();

            throw new NotImplementedException();
        }

        /// <summary>
        /// Gets or sets the element at the specified index.
        /// </summary>
        /// <returns>
        /// The element at the specified index.
        ///   </returns>
        ///   
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="index"/> is not a valid index in the <see cref="T:System.Collections.IList"/>.
        ///   </exception>
        ///   
        /// <exception cref="T:System.NotSupportedException">
        /// The property is set and the <see cref="T:System.Collections.IList"/> is read-only.
        ///   </exception>
        public Object this[Int32[] indexes]
        {
            get
            {
                MessageHandler.DebugEntry();
                MessageHandler.DebugEnhancedFormat("indexes=[{0}]", IndexesToString(indexes));

                Object value = GetValue(indexes);

                MessageHandler.DebugExit(value);

                return value;
            }
            set
            {
                MessageHandler.DebugEntry();
                MessageHandler.DebugEnhancedFormat("indexes=[{0}], value={1}", IndexesToString(indexes), value);

                SetValue(indexes, value);

                MessageHandler.DebugExit();
            }
        }

        /// <summary>
        /// Gets or sets the element at the specified index.
        /// </summary>
        /// <returns>
        /// The element at the specified index.
        ///   </returns>
        ///   
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///   <paramref name="index"/> is not a valid index in the <see cref="T:System.Collections.IList"/>.
        ///   </exception>
        ///   
        /// <exception cref="T:System.NotSupportedException">
        /// The property is set and the <see cref="T:System.Collections.IList"/> is read-only.
        ///   </exception>
        public Object this[Int32 index]
        {
            get
            {
                MessageHandler.DebugEntry();
                MessageHandler.DebugEnhancedFormat("index=[{0}]", index);

                Object value = GetValue(new Int32[] { index });

                MessageHandler.DebugExit(value);

                return value;
            }
            set
            {
                MessageHandler.DebugEntry();
                MessageHandler.DebugEnhancedFormat("index={0}, value={1}", index, value);

                SetValue(new Int32[] { index }, value);

                MessageHandler.DebugExit();
            }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="SparseArray"/> class.
        /// </summary>
        /// <param name="dimensions">The number of dimensions.</param>
        public SparseArray(Int32 dimensions)
        {
            MessageHandler.DebugEntry();
            MessageHandler.DebugEnhancedFormat("dimensions={0}", dimensions);

            mDimensions = dimensions;

            mValues = new Dictionary<String, Object>();

            // Handle scalar
            if (dimensions > 0)
            {
                mLowerBounds = new Int32[dimensions];
                mUpperBounds = new Int32[dimensions];
            }

            MessageHandler.DebugExit();
        }

        #endregion

    }
}
