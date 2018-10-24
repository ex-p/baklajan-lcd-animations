import numpy as np
import os


def save_right(matrix, file_name):
    with open(file_name, 'w') as f:
        for i in range(matrix.shape[0]):
            for j in range(matrix.shape[1]):
                jdx = j if i % 2 == 0 else matrix.shape[1] - j - 1
                for k in range(matrix.shape[2]):
                    f.write('{}\n'.format(matrix[i][jdx][k]))


def save_left(matrix, file_name):
    with open(file_name, 'w') as f:
        for i in range(matrix.shape[0]):
            idx = matrix.shape[0] - 1 - i
            for j in range(matrix.shape[1]):
                jdx = matrix.shape[1] - 1 - j if idx % 2 == 0 else j
                for k in range(matrix.shape[2]):
                    f.write('{}\n'.format(matrix[idx][jdx][k]))


def save(matrix, left_len, animation, stage):
    if not os.path.exists(animation):
        os.makedirs(animation)

    matrix = np.array(matrix)
    left_file = '{}/l{}.txt'.format(animation, stage)
    right_file = '{}/r{}.txt'.format(animation, stage)
    save_left(matrix[:, :left_len], left_file)
    save_right(matrix[:, left_len:], right_file)


test = [
    [
        [1, 2, 3], [4, 5, 6],  [19, 11, 12], [10, 11, 12]
    ],
    [
        [7, 8, 9], [10, 11, 12], [13, 14, 15], [10, 11, 12]
    ],
    [
        [13, 14, 15], [16, 17, 18],  [16, 17, 18], [10, 11, 12]
    ],
]


def generate_outer(rows, cols, left_len, right_len, ac_color, bg_color):
    grid = np.zeros((rows, cols, 3), dtype=int)
    for stage in range(left_len):
        grid[:, :] = np.array(bg_color)
        edge = int(round((stage + 1) / left_len * right_len))
        for row in range(rows):
            for i in range(stage + 1):
                idx = left_len - 1 - i
                grid[row][idx] = ac_color
            for i in range(edge):
                idx = left_len + i
                grid[row][idx] = ac_color
        yield grid


def main():
    rows = 3

    # be aware left len >= right len
    left_len = 16  # 300
    right_len = 8  # 192

    cols = left_len + right_len

    purple = [94, 0, 211]
    green = [0, 255, 0]
    gen = generate_outer(rows, cols, left_len, right_len, green, purple)
    for stage, grid in enumerate(gen):
        save(grid, left_len, 'outer', stage)


if __name__ == "__main__":
    main()
