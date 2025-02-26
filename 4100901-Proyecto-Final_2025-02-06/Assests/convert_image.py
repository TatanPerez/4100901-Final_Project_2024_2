from PIL import Image
import numpy as np

def convert_png_to_c_array(image_path, output_file, width=128, height=64):
    """
    Converts a PNG image to a C array in ROW-MAJOR order for ssd1306_DrawBitmap().
    Each byte represents 8 horizontal pixels.
    """
    img = Image.open(image_path).convert('1')  # Convert to monochrome (1-bit)
    img = img.resize((width, height))  # Resize image if necessary

    img_data = np.array(img)

    byte_width = (width + 7) // 8  # Number of bytes per row
    byte_array = [0] * (byte_width * height)

    for y in range(height):
        for x in range(width):
            byte_index = y * byte_width + (x // 8)  # Position in byte array
            bit_position = 7 - (x % 8)  # MSB first


            if img_data[y, x] == 0:  # Black pixel (ON)
                byte_array[byte_index] |= (1 << bit_position)

    # Generate C array output
    c_code = f"const uint8_t {output_file}[{len(byte_array)}] = {{\n"
    for i, byte in enumerate(byte_array):
        c_code += f" 0x{byte:02X},"
        if (i + 1) % 16 == 0:  # New line every 16 bytes
            c_code += "\n"
    c_code += "\n};\n"

    # Save to file
    with open(output_file, 'w') as f:
        f.write(c_code)

    print(f"C array saved to {output_file}")

# Example usage
convert_png_to_c_array(r"C:\Users\Tatan Perez\Documents\Unal\Tuition 6\Estructuras Computacionales\GithubDesktop\4100901-Final_Project_2024_2\4100901-Proyecto-Final_2025-02-06\Assests\texto_open-r.png", "open_text_r.h", width=128, height=64)
