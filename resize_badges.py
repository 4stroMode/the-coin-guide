import os
from PIL import Image

def resize_image(image_path, target_width_uhd=None):
    img = Image.open(image_path)
    base_name, ext = os.path.splitext(image_path)
    
    if target_width_uhd is None:
        target_width_uhd = img.width

    img_uhd = img.resize((target_width_uhd, int(img.height * (target_width_uhd / img.width))), Image.LANCZOS)
    img_uhd.save(f"{base_name}-uhd{ext}")

    img_hd = img_uhd.resize((target_width_uhd // 2, img_uhd.height // 2), Image.LANCZOS)
    img_hd.save(f"{base_name}-hd{ext}")

    img_low = img_uhd.resize((target_width_uhd // 4, img_uhd.height // 4), Image.LANCZOS)
    img_low.save(f"{base_name}{ext}")

resize_image('resources/adminbadge.png', 75)
resize_image('resources/modbadge.png', 75)
print("Resized successfully.")
