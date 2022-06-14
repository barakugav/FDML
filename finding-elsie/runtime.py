from runtime_single_measurement import *
from runtime_mia import *

if __name__ == "__main__":
    # ds = [0.69, 2.06, 3.45, 1.46]
    # ds = [0.48, 0.73, 1.98, 0.73]
    ds = [0.47, 0.59, 3.11, 0.32]
    alphas = [0, math.pi / 2, math.pi, 3 * math.pi / 2]

    N = 100

    for i in range(4):
        single_measurement(
            polygon_filename="data/lab.poly",
            output_filename="data/manifolds/m3_{}.obj".format(i+1),
            N=N,
            d=ds[i],
            alpha=alphas[i],
            manifold_image="data/images/m3_{}.png".format(i+1)
        )

    run_mia(N, 1.0, 2.0)