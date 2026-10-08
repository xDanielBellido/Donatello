using System.Runtime.InteropServices;

namespace Donatello.App;

internal static class NativeMethods
{
    [DllImport(
        "Donatello.Native.dll",
        EntryPoint = "Donatello_GetApiVersion",
        CallingConvention = CallingConvention.Cdecl,
        ExactSpelling = true)]
    internal static extern int GetApiVersion();
}