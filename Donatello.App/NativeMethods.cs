using System.Runtime.InteropServices;
using System.Text;

namespace Donatello.App;

internal static class NativeMethods
{
    [DllImport("Donatello.Native.dll", EntryPoint = "Donatello_GetApiVersion", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
    internal static extern int GetApiVersion();

    [DllImport("Donatello.Native.dll", EntryPoint = "Donatello_CreateGraphics", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
    internal static extern int CreateGraphics(out GraphicsSafeHandle graphics);

    [DllImport("Donatello.Native.dll", EntryPoint = "Donatello_DestroyGraphics", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
    internal static extern void DestroyGraphics(IntPtr graphics);

    [DllImport("Donatello.Native.dll", EntryPoint = "Donatello_GetGraphicsAdapterName", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, ExactSpelling = true)]
    internal static extern int GetGraphicsAdapterName(GraphicsSafeHandle graphics, [Out] StringBuilder buffer, uint capacity);

}