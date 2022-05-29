def load_polygon(filename):
    polygon = []
    with open(filename, 'r') as fp:
        for line in fp.readlines():
            x, y = line.split()
            polygon.append((float(x), float(y)))
    return polygon

if __name__ == "__main__":
    filename = "meshing/resources/polygons/checkpoint.poly"
    polygon = []
    with open(filename, 'r') as fp:
        with open("meshing/resources/polygons/checkpoint_normalized.poly", 'w') as fp2:
            for line in fp.readlines():
                x, y = line.split()
                fp2.write("{} {}\n".format(float(x) / 20.0, float(y) / 20.0))
                polygon.append((float(x), float(y)))
