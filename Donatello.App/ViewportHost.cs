/*
    ===========================================================================
    ViewportHost
    ===========================================================================

    Hosts a native child window inside the WPF interface.

    Creates and destroys the window that will receive DirectX 12 presentation.
    WPF manages its position and size within the application layout.

    Currently displays a black Win32 control without DirectX rendering.
*/

using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Interop;

namespace Donatello.App;

public sealed class ViewportHost : HwndHost
{
    private const uint WS_CHILD = 0x40000000;
    private const uint WS_VISIBLE = 0x10000000;
    private const uint SS_BLACKRECT = 0x00000004;

    protected override HandleRef BuildWindowCore(HandleRef hwndParent)
    {
        uint style = WS_CHILD | WS_VISIBLE | SS_BLACKRECT;
        IntPtr window = CreateWindowExW(0, "STATIC", "", style, 0, 0, 1, 1, hwndParent.Handle, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero);

        if (window == IntPtr.Zero)
        {
            throw new Win32Exception(Marshal.GetLastWin32Error());
        }

        Debug.WriteLine("Donatello: native viewport window created.");

        return new HandleRef(this, window);
    }

    protected override void DestroyWindowCore(HandleRef hwnd)
    {
        if (!DestroyWindow(hwnd.Handle))
        {
            int error = Marshal.GetLastWin32Error();
            Debug.WriteLine($"Donatello: viewport window destruction failed with Win32 error {error}.");
        }
    }


    // Call native Win32 APIs to create and destroy the child window. 
    [DllImport("user32.dll", CharSet = CharSet.Unicode, ExactSpelling = true, SetLastError = true)]
    private static extern IntPtr CreateWindowExW(uint extendedStyle, string className, string windowName, uint style, int x, int y, int width, int height, IntPtr parent, IntPtr menu, IntPtr instance, IntPtr parameter);

    [DllImport("user32.dll", ExactSpelling = true, SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool DestroyWindow(IntPtr window);
}