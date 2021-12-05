using System;
using System.Runtime.InteropServices;

namespace ComUtilityManaged
{
    /// <summary>
    /// Wrapper for IAgileReference.
    /// </summary>
    /// <typeparam name="T"> interface </typeparam>
    public class AgileReferenceWrapper<T> where T : class
    {
        /// <summary> Thread safe agile reference </summary>
        IAgileReference m_agileReference;

        /// <summary>
        /// Create AgileReferenceWrapper from COM object.
        /// </summary>
        /// <param name="toMarshal"> object to marshal </param>
        public AgileReferenceWrapper(T toMarshal)
        {
            NativeMethods.RoGetAgileReference((int)NativeMethods.AgileReferenceOptions.AGILEREFERENCE_DEFAULT, typeof(T).GUID, toMarshal, out IntPtr reference);
            m_agileReference = Marshal.GetObjectForIUnknown(reference) as IAgileReference;
        }

        /// <summary>
        /// Get marshalled COM object.
        /// </summary>
        /// <returns></returns>
        public T Resolve()
        {
            return m_agileReference.Resolve<T>();
        }
    }

    #region Native implementation details
    public static class NativeMethods
    {
        public enum AgileReferenceOptions
        {
            AGILEREFERENCE_DEFAULT,
            AGILEREFERENCE_DELAYEDMARSHAL
        };

        [DllImport("ole32.dll", PreserveSig = false)]
        public static extern void RoGetAgileReference(
            int options,
            [In, MarshalAs(UnmanagedType.LPStruct)] Guid riid,
            [In, MarshalAs(UnmanagedType.IUnknown)] object pUnk,
            out IntPtr ar);
    }

    [ComImport]
    [Guid("C03F6A43-65A4-9818-987E-E0B810D2A6F2")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAgileReference
    {
        [return: MarshalAs(UnmanagedType.Interface)]
        object Resolve([In, MarshalAs(UnmanagedType.LPStruct)] Guid riid);
    }

    public static class IAgileReferenceExt
    {
        public static T Resolve<T>(this IAgileReference me)
        {
            return (T)me.Resolve(typeof(T).GUID);
        }
    }
    #endregion
}
