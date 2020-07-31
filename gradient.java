import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.image.*;
import javax.swing.JFrame;
import javax.swing.JPanel;

public class show extends JPanel {
    public static final int WINDOW_W = 640;
    public static final int WINDOW_H = 480;
    public static final int W = 256;
    public static final int H = 256;

    private BufferedImage image;

    private show(){
        // Allocate a surface (BufferedImage) describing a 24-bit raw bitmap.
        image = new BufferedImage(W, H, BufferedImage.TYPE_3BYTE_BGR);
        byte[] gradient = 
        ((DataBufferByte)image.getRaster().getDataBuffer()).getData();

        //
    }
}