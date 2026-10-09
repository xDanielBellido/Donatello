using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Donatello.App
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private GraphicsSafeHandle? graphics;

        public MainWindow()
        {
            InitializeComponent();

            int nativeVersion = NativeMethods.GetApiVersion();
            int result = NativeMethods.CreateGraphics(out GraphicsSafeHandle createdGraphics);

            if (result < 0)
            {
                createdGraphics.Dispose();
                Title = $"Donatello — Graphics initialisation failed: 0x{result:X8}";
                return;
            }

            graphics = createdGraphics;
            Viewport.Loaded += OnViewportLoaded;

            var adapterName = new StringBuilder(128);
            result = NativeMethods.GetGraphicsAdapterName(graphics, adapterName, (uint)adapterName.Capacity);

            if (result < 0)
            {
                Title = $"Donatello — DirectX 12 ready — Adapter name unavailable: 0x{result:X8}";
                return;
            }

            Title = $"Donatello — Native API v{nativeVersion} — {adapterName.ToString()}";
        }

        private void OnViewportLoaded(object sender, RoutedEventArgs e)
        {
            Viewport.Loaded -= OnViewportLoaded;

            if (graphics is null || graphics.IsInvalid || graphics.IsClosed)
            {
                return;
            }

            if (Viewport.Handle == IntPtr.Zero)
            {
                Title = "Donatello — Native viewport handle is unavailable";
                return;
            }

            int result = NativeMethods.CreateSwapChain(graphics, Viewport.Handle);

            if (result < 0)
            {
                Title = $"Donatello — Swap chain initialisation failed: 0x{result:X8}";
                return;
            }

            result = NativeMethods.RenderFrame(graphics);

            if (result < 0)
            {
                Title = $"Donatello — First frame failed: 0x{result:X8}";
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            try
            {
                Viewport.Loaded -= OnViewportLoaded;

                graphics?.Dispose();
                graphics = null;

                Viewport.Dispose();
            }
            finally
            {
                base.OnClosed(e);
            }
        }

    }
}