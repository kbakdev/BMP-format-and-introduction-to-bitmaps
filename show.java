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

        // Draw a red-black gradient in BGR space.
        for (int j = 0; j < H; j++) {
            for (int i = 0; i < W; i++) {
                gradient[(j * W + i) * 3 + 2] = (byte)i;
                gradient[(j * W + i) * 3 + 1] = 0;
                gradient[(j * W + i) * 3 + 0] = 0;
            }
        }
    }

    // Redraw the bitmap onto a caller supplied framebuffer.
    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        g.drawImage(image, (WINDOW_W - W) / 2, (WINDOW_H - H) / 2, null);
    }

    @Override
    public Dimension getPreferredSize() {
        return new Dimension(WINDOW_W, WINDOW_H);
    }

    public static void main(String[] args) {
        // Create a window and display it.
        JFrame frame = new JFrame("show");
        frame.add(new show());
        frame.pack();
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setVisible(true);
    }
}