//------------------------------------------------------------------------------
//
// <copyright file="ListEv.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;

// extend System.Collections.Generic
namespace System.Collections.Generic
{
    /// <summary>
    /// Wraps a List&lt;T&gt; which can contain items of specified type (T).
    /// Methods which change the list contents fire "Before..." and "After..." events associated with them.
    /// The "Before..." events are cancellable.
    /// </summary>
    /// <typeparam name="T">The type of the items in the list.</typeparam>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1710:IdentifiersShouldHaveCorrectSuffix"), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Ev"), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Ev")]
    public class ListEv<T>
        : List<T>
    {
        #region EventArgs

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class OldNewItemEventArgs
            : EventArgs
        {
            private readonly T mOldValue;

            /// <summary>
            /// Gets the old value.
            /// </summary>
            public T OldValue
            {
                get
                {
                    return mOldValue;
                }
            }

            private T mNewValue;

            /// <summary>
            /// Gets or sets the new value.
            /// </summary>
            /// <value>
            /// The new value.
            /// </value>
            public T NewValue
            {
                get
                {
                    return mNewValue;
                }
                set
                {
                    mNewValue = value;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="OldNewItemEventArgs"/> class.
            /// </summary>
            /// <param name="oldValue">The old value.</param>
            /// <param name="newValue">The new value.</param>
            public OldNewItemEventArgs(T oldValue, T newValue)
            {
                mOldValue = oldValue;
                mNewValue = newValue;
            }

            /// <summary>
            /// Prevents a default instance of the <see cref="AfterOldNewItemEventArgs"/> class from being created.
            /// </summary>
            private OldNewItemEventArgs()
            {
            }
        }

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class CancelOldNewItemEventArgs
            : CancelEventArgs
        {
            private readonly T mOldValue;

            /// <summary>
            /// Gets the old value.
            /// </summary>
            public T OldValue
            {
                get
                {
                    return mOldValue;
                }
            }

            private T mNewValue;

            /// <summary>
            /// Gets or sets the new value.
            /// </summary>
            /// <value>
            /// The new value.
            /// </value>
            public T NewValue
            {
                get
                {
                    return mNewValue;
                }
                set
                {
                    mNewValue = value;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelOldNewItemEventArgs"/> class.
            /// </summary>
            /// <param name="oldValue">The old value.</param>
            /// <param name="newValue">The new value.</param>
            public CancelOldNewItemEventArgs(T oldValue, T newValue)
                : base()
            {
                mOldValue = oldValue;
                mNewValue = newValue;
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelOldNewItemEventArgs"/> class.
            /// </summary>
            /// <param name="oldValue">The old value.</param>
            /// <param name="newValue">The new value.</param>
            /// <param name="cancel">if set to <c>true</c> [cancel].</param>
            public CancelOldNewItemEventArgs(T oldValue, T newValue, Boolean cancel)
                : base(cancel)
            {
                mOldValue = oldValue;
                mNewValue = newValue;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class ItemEventArgs
            : EventArgs
        {
            private readonly Int32 mIndex;

            /// <summary>
            /// Gets the index.
            /// </summary>
            /// <value>
            /// The index.
            /// </value>
            public Int32 Index
            {
                get
                {
                    return mIndex;
                }
            }

            private readonly T mItem;

            /// <summary>
            /// Gets the item.
            /// </summary>
            /// <value>
            /// The item.
            /// </value>
            public T Item
            {
                get
                {
                    return mItem;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="ItemEventArgs"/> class.
            /// </summary>
            /// <param name="index">The item's index.</param>
            /// <param name="item">The item.</param>
            public ItemEventArgs(Int32 index, T item)
            {
                mIndex = index;
                mItem = item;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class CancelItemEventArgs
            : CancelEventArgs
        {
            private readonly Int32 mIndex;

            /// <summary>
            /// Gets the index.
            /// </summary>
            /// <value>
            /// The index.
            /// </value>
            public Int32 Index
            {
                get
                {
                    return mIndex;
                }
            }

            private T mItem;

            /// <summary>
            /// Gets or sets the item.
            /// </summary>
            /// <value>
            /// The item.
            /// </value>
            public T Item
            {
                get
                {
                    return mItem;
                }
                set
                {
                    mItem = value;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelItemEventArgs"/> class.
            /// </summary>
            /// <param name="index">The item's index.</param>
            /// <param name="item">The item.</param>
            public CancelItemEventArgs(Int32 index, T item)
                : base()
            {
                mIndex = index;
                mItem = item;
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelItemEventArgs"/> class.
            /// </summary>
            /// <param name="index">The item's index.</param>
            /// <param name="item">The item.</param>
            /// <param name="cancel">The cancel.</param>
            public CancelItemEventArgs(Int32 index, T item, Boolean cancel)
                : base(cancel)
            {
                mIndex = index;
                mItem = item;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class CollectionEventArgs
            : EventArgs
        {
            private readonly IEnumerable<T> mCollection;

            /// <summary>
            /// Gets the collection.
            /// </summary>
            /// <value>
            /// The collection.
            /// </value>
            public IEnumerable<T> Collection
            {
                get
                {
                    return mCollection;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CollectionEventArgs"/> class.
            /// </summary>
            /// <param name="collection">The collection.</param>
            public CollectionEventArgs(IEnumerable<T> collection)
            {
                mCollection = collection;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class CancelCollectionEventArgs
            : CancelEventArgs
        {
            private readonly IEnumerable<T> mCollection;

            /// <summary>
            /// Gets the collection.
            /// </summary>
            /// <value>
            /// The collection.
            /// </value>
            public IEnumerable<T> Collection
            {
                get
                {
                    return mCollection;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelCollectionEventArgs"/> class.
            /// </summary>
            /// <param name="collection">The collection.</param>
            public CancelCollectionEventArgs(IEnumerable<T> collection)
                : base()
            {
                mCollection = collection;
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelCollectionEventArgs"/> class.
            /// </summary>
            /// <param name="collection">The collection.</param>
            /// <param name="cancel">The cancel.</param>
            public CancelCollectionEventArgs(IEnumerable<T> collection, Boolean cancel)
                : base(cancel)
            {
                mCollection = collection;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class RangeEventArgs
            : EventArgs
        {
            private readonly Int32 mStart;

            /// <summary>
            /// Gets the start.
            /// </summary>
            /// <value>
            /// The start.
            /// </value>
            public Int32 Start
            {
                get
                {
                    return mStart;
                }
            }

            private readonly Int32 mCount;

            /// <summary>
            /// Gets the count.
            /// </summary>
            /// <value>
            /// The count.
            /// </value>
            public Int32 Count
            {
                get
                {
                    return mCount;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="RangeEventArgs"/> class.
            /// </summary>
            /// <param name="start">The start.</param>
            /// <param name="count">The count.</param>
            public RangeEventArgs(Int32 start, Int32 count)
            {
                mStart = start;
                mCount = count;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1034:NestedTypesShouldNotBeVisible")]
        public class CancelRangeEventArgs
            : CancelEventArgs
        {
            private readonly Int32 mStart;

            /// <summary>
            /// Gets the start.
            /// </summary>
            /// <value>
            /// The start.
            /// </value>
            public Int32 Start
            {
                get
                {
                    return mStart;
                }
            }

            private readonly Int32 mCount;

            /// <summary>
            /// Gets the count.
            /// </summary>
            /// <value>
            /// The count.
            /// </value>
            public Int32 Count
            {
                get
                {
                    return mCount;
                }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelRangeEventArgs"/> class.
            /// </summary>
            /// <param name="start">The start.</param>
            /// <param name="count">The count.</param>
            public CancelRangeEventArgs(Int32 start, Int32 count)
                : base()
            {
                mStart = start;
                mCount = count;
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="CancelRangeEventArgs"/> class.
            /// </summary>
            /// <param name="start">The start.</param>
            /// <param name="count">The count.</param>
            /// <param name="cancel">The cancel.</param>
            public CancelRangeEventArgs(Int32 start, Int32 count, Boolean cancel)
                : base(cancel)
            {
                mStart = start;
                mCount = count;
            }
        }

        #endregion

        #region Events

        /// <summary>
        /// Occurs before an item is set.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelOldNewItemEventArgs> BeforeSet;

        /// <summary>
        /// Raises the <see cref="E:BeforeSet" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CancelOldNewItemEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeSet(CancelOldNewItemEventArgs e)
        {
            if (BeforeSet != null)
            {
                BeforeSet(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after an item is set.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<OldNewItemEventArgs> AfterSet;

        /// <summary>
        /// Raises the <see cref="E:AfterSet" /> event.
        /// </summary>
        /// <param name="e">The <see cref="OldNewItemEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterSet(OldNewItemEventArgs e)
        {
            if (AfterSet != null)
            {
                AfterSet(this, e);
            }
        }

        /// <summary>
        /// Occurs after the item is 'got'.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<ItemEventArgs> AfterGet;

        /// <summary>
        /// Raises the <see cref="E:AfterGet" /> event.
        /// </summary>
        /// <param name="e">The <see cref="ItemEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterGet(ItemEventArgs e)
        {
            if (AfterGet != null)
            {
                AfterGet(this, e);
            }
        }

        /// <summary>
        /// Occurs before the list is cleared.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelEventArgs> BeforeClear;

        /// <summary>
        /// Raises the <see cref="E:BeforeClear"/> event.
        /// </summary>
        /// <param name="e">The <see cref="System.ComponentModel.CancelEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeClear(CancelEventArgs e)
        {
            if (BeforeClear != null)
            {
                BeforeClear(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after the list is cleared.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler AfterClear;

        /// <summary>
        /// Raises the <see cref="E:AfterClear"/> event.
        /// </summary>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterClear(EventArgs e)
        {
            if (AfterClear != null)
            {
                AfterClear(this, e);
            }
        }

        /// <summary>
        /// Occurs before an item is added.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelItemEventArgs> BeforeAdd;

        /// <summary>
        /// Raises the <see cref="E:BeforeAdd" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CancelItemEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeAdd(CancelItemEventArgs e)
        {
            if (BeforeAdd != null)
            {
                BeforeAdd(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after an item is added.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<ItemEventArgs> AfterAdd;

        /// <summary>
        /// Raises the <see cref="E:AfterAdd" /> event.
        /// </summary>
        /// <paListEve">The <see cref="ItemEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterAdd(ItemEventArgs e)
        {
            if (AfterAdd != null)
            {
                AfterAdd(this, e);
            }
        }

        /// <summary>
        /// Occurs before an item is inserted.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelItemEventArgs> BeforeInsert;

        /// <summary>
        /// Raises the <see cref="E:BeforeInsert" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CancelItemEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeInsert(CancelItemEventArgs e)
        {
            if (BeforeInsert != null)
            {
                BeforeInsert(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after an item is inserted.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<ItemEventArgs> AfterInsert;

        /// <summary>
        /// Raises the <see cref="E:AfterInsert" /> event.
        /// </summary>
        /// <param name="e">The <see cref="ItemEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterInsert(ItemEventArgs e)
        {
            if (AfterInsert != null)
            {
                AfterInsert(this, e);
            }
        }

        /// <summary>
        /// Occurs before an item is removed.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelItemEventArgs> BeforeRemove;

        /// <summary>
        /// Raises the <see cref="E:BeforeRemove" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CancelItemEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeRemove(CancelItemEventArgs e)
        {
            if (BeforeRemove != null)
            {
                BeforeRemove(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after an item is removed.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<ItemEventArgs> AfterRemove;

        /// <summary>
        /// Raises the <see cref="E:AfterRemove" /> event.
        /// </summary>
        /// <param name="e">The <see cref="ItemEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterRemove(ItemEventArgs e)
        {
            if (AfterRemove != null)
            {
                AfterRemove(this, e);
            }
        }

        /// <summary>
        /// Occurs before a range of items is added.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelCollectionEventArgs> BeforeAddRange;

        /// <summary>
        /// Raises the <see cref="E:BeforeAddRange" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CancelCollectionEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeAddRange(CancelCollectionEventArgs e)
        {
            if (BeforeAddRange != null)
            {
                BeforeAddRange(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after a range of items is added.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CollectionEventArgs> AfterAddRange;

        /// <summary>
        /// Raises the <see cref="E:AfterAddRange" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CollectionEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterAddRange(CollectionEventArgs e)
        {
            if (AfterAddRange != null)
            {
                AfterAddRange(this, e);
            }
        }

        /// <summary>
        /// Occurs before a range of items is inserted.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelCollectionEventArgs> BeforeInsertRange;

        /// <summary>
        /// Raises the <see cref="E:BeforeInsertRange" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CancelInsertRangeEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeInsertRange(CancelCollectionEventArgs e)
        {
            if (BeforeInsertRange != null)
            {
                BeforeInsertRange(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after a range of items is inserted.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CollectionEventArgs> AfterInsertRange;

        /// <summary>
        /// Raises the <see cref="E:AfterInsertRange" /> event.
        /// </summary>
        /// <param name="e">The <see cref="InsertRangeEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterInsertRange(CollectionEventArgs e)
        {
            if (AfterInsertRange != null)
            {
                AfterInsertRange(this, e);
            }
        }

        /// <summary>
        /// Occurs before a range of items is removed.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<CancelRangeEventArgs> BeforeRemoveRange;

        /// <summary>
        /// Raises the <see cref="E:BeforeRemoveRange" /> event.
        /// </summary>
        /// <param name="e">The <see cref="CancelRangeEventArgs"/> instance containing the event data.</param>
        /// <returns><c>true</c> if the action is to be cancelled.</returns>
        protected virtual Boolean OnBeforeRemoveRange(CancelRangeEventArgs e)
        {
            if (BeforeRemoveRange != null)
            {
                BeforeRemoveRange(this, e);
            }

            return e.Cancel;
        }

        /// <summary>
        /// Occurs after a range of items is removed.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1713:EventsShouldNotHaveBeforeOrAfterPrefix")]
        public event EventHandler<RangeEventArgs> AfterRemoveRange;

        /// <summary>
        /// Raises the <see cref="E:AfterRemoveRange" /> event.
        /// </summary>
        /// <param name="e">The <see cref="RangeEventArgs"/> instance containing the event data.</param>
        protected virtual void OnAfterRemoveRange(RangeEventArgs e)
        {
            if (AfterRemoveRange != null)
            {
                AfterRemoveRange(this, e);
            }
        }

        /// <summary>
        /// Occurs after the number of items in the list has changed.
        /// </summary>
        public event EventHandler<EventArgs> CountChanged;

        /// <summary>
        /// Raises the <see cref="E:CountChanged" /> event.
        /// </summary>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        protected virtual void OnCountChanged(EventArgs e)
        {
            if (CountChanged != null)
            {
                CountChanged(this, e);
            }
        }

        #endregion

        #region List wrapper

        /// <summary>
        /// Gets or sets the element at the specified index.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <returns>The element at the specified index.</returns>
        public new T this[Int32 index]
        {
            get
            {
                T item = base[index];

                OnAfterGet(new ItemEventArgs(index, item));

                return item;
            }
            set
            {
                T oldValue = base[index];

                CancelOldNewItemEventArgs e = new CancelOldNewItemEventArgs(oldValue, value);
                if (OnBeforeSet(e))
                {
                    return;
                }

                base[index] = e.NewValue;

                OnAfterSet(new OldNewItemEventArgs(oldValue, value));
            }
        }

        /// <summary>
        /// Adds an object to the end of the <see cref="T:System.Collections.Generic.List`1" />.
        /// </summary>
        /// <param name="item">The object to be added to the end of the <see cref="T:System.Collections.Generic.List`1" />. The value can be null for reference types.</param>
        public new void Add(T item)
        {
            Int32 index = base.IndexOf(item);
            CancelItemEventArgs e = new CancelItemEventArgs(index, item);
            if (OnBeforeAdd(e))
            {
                return;
            }

            base.Add(e.Item);

            OnAfterAdd(new ItemEventArgs(index, e.Item));
            OnCountChanged(EventArgs.Empty);
        }

        /// <summary>
        /// Adds the elements of the specified collection to the end of the <see cref="T:System.Collections.Generic.List`1" />.
        /// </summary>
        /// <param name="collection">The collection whose elements should be added to the end of the <see cref="T:System.Collections.Generic.List`1" />. The collection itself cannot be null, but it can contain elements that are null, if type <paramref name="T" /> is a reference type.</param>
        public new void AddRange(IEnumerable<T> collection)
        {
            CancelCollectionEventArgs e = new CancelCollectionEventArgs(collection);
            if (OnBeforeAddRange(e))
            {
                return;
            }

            base.AddRange(e.Collection);

            OnAfterAddRange(new CollectionEventArgs(e.Collection));
            OnCountChanged(EventArgs.Empty);
        }

        /// <summary>
        /// Removes all elements from the <see cref="T:System.Collections.Generic.List`1" />.
        /// </summary>
        public new void Clear()
        {
            if (OnBeforeClear(new CancelEventArgs()))
            {
                return;
            }
            
            base.Clear();

            OnAfterClear(EventArgs.Empty);
            OnCountChanged(EventArgs.Empty);
        }

        /// <summary>
        /// Inserts an element into the <see cref="T:System.Collections.Generic.List`1" /> at the specified index.
        /// </summary>
        /// <param name="index">The zero-based index at which <paramref name="item" /> should be inserted.</param>
        /// <param name="item">The object to insert. The value can be null for reference types.</param>
        public new void Insert(Int32 index, T item)
        {
            CancelItemEventArgs e = new CancelItemEventArgs(index, item);
            if (OnBeforeInsert(e))
            {
                return;
            }

            base.Insert(index, e.Item);

            OnAfterInsert(new ItemEventArgs(index, e.Item));
            OnCountChanged(EventArgs.Empty);
        }

        /// <summary>
        /// Inserts the elements of a collection into the <see cref="T:System.Collections.Generic.List`1" /> at the specified index.
        /// </summary>
        /// <param name="index">The zero-based index at which the new elements should be inserted.</param>
        /// <param name="collection">The collection whose elements should be inserted into the <see cref="T:System.Collections.Generic.List`1" />. The collection itself cannot be null, but it can contain elements that are null, if type <paramref name="T" /> is a reference type.</param>
        public new void InsertRange(Int32 index, IEnumerable<T> collection)
        {
            CancelCollectionEventArgs e = new CancelCollectionEventArgs(collection);
            if (OnBeforeInsertRange(e))
            {
                return;
            }

            base.InsertRange(index, e.Collection);

            OnAfterInsertRange(new CollectionEventArgs(e.Collection));
            OnCountChanged(EventArgs.Empty);
        }

        /// <summary>
        /// Removes the first occurrence of a specific object from the <see cref="T:System.Collections.Generic.List`1" />.
        /// </summary>
        /// <param name="item">The object to remove from the <see cref="T:System.Collections.Generic.List`1" />. The value can be null for reference types.</param>
        /// <returns>
        /// true if <paramref name="item" /> is successfully removed; otherwise, false.  This method also returns false if <paramref name="item" /> was not found in the <see cref="T:System.Collections.Generic.List`1" />.
        /// </returns>
        public new bool Remove(T item)
        {
            Int32 index = IndexOf(item);
            if (index >= 0)
            {
                if (OnBeforeRemove(new CancelItemEventArgs(index, item)))
                {
                    return false;
                }

                base.Remove(item);

                OnAfterRemove(new ItemEventArgs(index, item));
                OnCountChanged(EventArgs.Empty);

                return true;
            }
            return false;
        }

        /// <summary>
        /// Removes the element at the specified index of the <see cref="T:System.Collections.Generic.List`1" />.
        /// </summary>
        /// <param name="index">The zero-based index of the element to remove.</param>
        public new void RemoveAt(Int32 index)
        {
            T item = base[index];

            if (OnBeforeRemove(new CancelItemEventArgs(index, item)))
            {
                return;
            }

            base.RemoveAt(index);

            OnAfterRemove(new ItemEventArgs(index, item));
            OnCountChanged(EventArgs.Empty);
        }

        /// <summary>
        /// Removes a range of elements from the <see cref="T:System.Collections.Generic.List`1" />.
        /// </summary>
        /// <param name="index">The zero-based starting index of the range of elements to remove.</param>
        /// <param name="count">The number of elements to remove.</param>
        public new void RemoveRange(Int32 index, Int32 count)
        {
            if (count > 0)
            {
                if (OnBeforeRemoveRange(new CancelRangeEventArgs(index, count)))
                {
                    return;
                }

                base.RemoveRange(index, count);

                OnAfterRemoveRange(new RangeEventArgs(index, count));
                OnCountChanged(EventArgs.Empty);
            }
        }

        #endregion

        #region Constructors etc.

        /// <summary>
        /// Initializes a new instance of the <see cref="ListEv{T}"/> class.
        /// </summary>
        public ListEv()
            : base()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ListEv{T}"/> class.
        /// </summary>
        /// <param name="capacity">The capacity.</param>
        /// <exception cref="System.ArgumentOutOfRangeException"></exception>
        public ListEv(Int32 capacity)
            : base(capacity)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ListEv{T}"/> class.
        /// </summary>
        /// <param name="collection">The collection.</param>
        public ListEv(IEnumerable<T> collection)
            : base(collection)
        {
        }

        #endregion

    }
}
