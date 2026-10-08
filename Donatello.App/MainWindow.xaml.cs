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

            var adapterName = new StringBuilder(128);
            result = NativeMethods.GetGraphicsAdapterName(graphics, adapterName, (uint)adapterName.Capacity);

            if (result < 0)
            {
                Title = $"Donatello — DirectX 12 ready — Adapter name unavailable: 0x{result:X8}";
                return;
            }

            Title = $"Donatello — Native API v{nativeVersion} — {adapterName.ToString()}";
        }

        protected override void OnClosed(EventArgs e)
        {
            try
            {
                graphics?.Dispose();
                graphics = null;
            }
            finally
            {
                base.OnClosed(e);
            }
        }

    }
}