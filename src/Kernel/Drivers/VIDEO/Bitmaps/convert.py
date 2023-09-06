from PIL import Image

def image_to_c_array(img_path):
    # Abre a imagem e converte para RGB
    img = Image.open(img_path).convert('RGB')

    # Obtém a largura e altura da imagem
    width, height = img.size

    # Obtém os pixels da imagem
    pixels = list(img.getdata())

    # Cria a string para o array C
    c_array = f'unsigned char image[{height}][{width}][3] = {{\n'
    for y in range(height):
        c_array += '{'
        for x in range(width):
            r, g, b = pixels[y * width + x]
            c_array += f'{{{r}, {g}, {b}}}, '
        c_array = c_array[:-2]  # Remove a última vírgula e espaço
        c_array += '},\n'
    c_array = c_array[:-2]  # Remove a última vírgula e quebra de linha
    c_array += '};\n'

    return c_array

# Teste
path = input()
print(image_to_c_array(path))
