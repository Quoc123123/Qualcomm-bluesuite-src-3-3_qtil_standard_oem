//------------------------------------------------------------------------------
//
// <copyright file="GeneralConcreteClassProvider.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows.Forms;

// Extend System.ComponentModel
namespace System.ComponentModel
{
    // Here is our type description provider.  This is the same provider
    // as ConcreteClassProvider except that it uses the ConcreteClassAttribute
    // to find the concrete class.
    public class GeneralConcreteClassProvider
        : TypeDescriptionProvider
    {
        /// <summary>
        /// The abstract type.
        /// </summary>
        private Type mAbstractType;

        /// <summary>
        /// The concreate type.
        /// </summary>
        private Type mConcreteType;

        /// <summary>
        /// This method locates the abstract and concrete types we should be returning.
        /// </summary>
        /// <param name="objectType">Type of the object.</param>
        private void EnsureTypes(Type objectType)
        {
            if (mAbstractType == null)
            {
                Type searchType = objectType;
                while ((mAbstractType == null) && (searchType != null) && (searchType != typeof(Object)))
                {
                    // Get the first (if any) ConcreteClassAttribute
                    foreach (ConcreteClassAttribute cca in searchType.GetCustomAttributes(typeof(ConcreteClassAttribute), false))
                    {
                        mAbstractType = searchType;
                        mConcreteType = cca.ConcreteType;
                        break;
                    }
                    searchType = searchType.BaseType;
                }

                if (mAbstractType == null)
                {
                    // If this happens, it means that someone added this provider to a class but did not add a ConcreteTypeAttribute
                    throw new InvalidOperationException(String.Format(CultureInfo.CurrentCulture, "No ConcreteClassAttribute was found on {0} or any of its subtypes.", objectType));
                }
            }
        }

        /// <summary>
        /// Performs normal reflection against the given object with the given type.
        /// </summary>
        /// <param name="objectType">The type of object for which to retrieve the <see cref="T:System.Reflection.IReflect"/>.</param>
        /// <param name="instance">An instance of the type. Can be null.</param>
        /// <returns>
        /// A <see cref="T:System.Type"/>.
        /// </returns>
        /// <remarks>
        /// Tell anyone who reflects on us that the concrete form is the form to 
        /// reflect against, not the abstract form. This way, the designer does 
        /// not see an abstract class.
        /// </remarks>
        public override Type GetReflectionType(Type objectType, object instance)
        {
            EnsureTypes(objectType);
            if (objectType == mAbstractType)
            {
                return mConcreteType;
            }
            return base.GetReflectionType(objectType, instance);
        }


        /// <summary>
        /// Creates an object that can substitute for another data type.
        /// </summary>
        /// <param name="provider">An optional service provider.</param>
        /// <param name="objectType">The type of object to create. This parameter is never null.</param>
        /// <param name="argTypes">An optional array of types that represent the parameter types to be passed to the object's constructor. This array can be null or of zero length.</param>
        /// <param name="args">An optional array of parameter values to pass to the object's constructor.</param>
        /// <returns>
        /// The substitute <see cref="T:System.Object"/>.
        /// </returns>
        /// <remarks>
        /// If the designer tries to create an instance of AbstractForm, we override it here to create a concerete form instead.
        /// </remarks>
        public override object CreateInstance(IServiceProvider provider, Type objectType, Type[] argTypes, object[] args)
        {
            EnsureTypes(objectType);
            if (objectType == mAbstractType)
            {
                objectType = mConcreteType;
            }

            return base.CreateInstance(provider, objectType, argTypes, args);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="GeneralConcreteUserControlProvider"/> class.
        /// </summary>
        public GeneralConcreteClassProvider()
            : base(TypeDescriptor.GetProvider(typeof(UserControl)))
        {
        }
    }
}
