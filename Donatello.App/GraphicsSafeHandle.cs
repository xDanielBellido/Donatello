using System;
using Microsoft.Win32.SafeHandles;

namespace Donatello.App;

internal sealed class GraphicsSafeHandle : SafeHandleZeroOrMinusOneIsInvalid
{
    public GraphicsSafeHandle() : base(ownsHandle: true)
    {
    }

    protected override bool ReleaseHandle()
    {
        NativeMethods.DestroyGraphics(handle);
        return true;
    }
}
