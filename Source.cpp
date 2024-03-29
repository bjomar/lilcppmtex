#include <iostream>
#include <vector>
#include <fstream>

//threading libs
#include <thread>
#include <future>

using matrix = std::vector<std::vector<int>>;

void printm(matrix& m) {
	std::cout << "matrix is: " << m.size() << " x " << m[0].size() << std::endl;
	for (size_t i = 0; i < m.size(); i++)
	{
		for (size_t j = 0; j < m[0].size(); j++)
		{
			std::cout << "[" << m[i][j] << "]";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

static matrix operator * (matrix& m1, matrix& m2) {
	if (!(m1.size() == m2[0].size() && m2.size() == m1[0].size()))
		return matrix();

	matrix res;

	int b = 0;

	for (size_t i = 0; i < m1.size(); i++)
	{
		res.push_back(std::vector<int>());
		for (size_t j = 0; j < m2[0].size(); j++)
		{
			for (size_t k = 0; k < m1[0].size(); k++)
			{
				b += m1[i][k] * m2[k][j];
			}

			res[i].push_back(b);
			b = 0;
		}
	}

	return res;
}

matrix sub_matrix(matrix& m, int startRow, int endRow, int startCol, int endCol) {

	matrix subMat;

	for (size_t i = startRow; i < endRow; i++)
	{
		subMat.push_back(std::vector<int>());
		for (size_t j = startCol; j < endCol; j++)
		{
			subMat[subMat.size() - 1].push_back(m[i][j]);
		}
	}

	return subMat;
}

void app_col(matrix& dst, matrix& m) {
	if (dst.size() != m.size() && dst.size())
		return;

	if (dst.size() == 0)
		dst = matrix(m.size());

	for (size_t i = 0; i < dst.size(); i++)
	{
		for (size_t j = 0; j < m[0].size(); j++)
		{
			dst.at(i).push_back(m[i][j]);
		}
	}
}

void app_row(matrix& dst, matrix& m) {
	for (auto v : m)
		dst.push_back(v);
}

matrix merge_matrices(std::vector<matrix> matrices, int colappBreaker) {

	matrix merged;
	matrix b;

	int i = 0;
	for (matrix m : matrices) {
		if (i == colappBreaker) {
			app_row(merged, b);
			b = matrix();
			i = 0;
		}

		app_col(b, m);

		i++;
	}

	app_row(merged, b);

	return merged;
}

matrix matmul(matrix& m1, matrix& m2) {
	return m1 * m2;
}

// it is advised to use a number for threads that fulfills sqrt(threads) elemnt N
// we will expect the matresses to be nxm with 2|n and m, and m*n is possible
matrix multMultiThread(matrix& m1, matrix& m2, unsigned int threads) {

#ifdef STATIC_4_T
	matrix	r1 = sub_matrix(m1, 0, m1.size() / 2, 0, m1[0].size()),
		r2 = sub_matrix(m1, m1.size() / 2, m1.size(), 0, m1[0].size());

	matrix	l1 = sub_matrix(m2, 0, m2.size(), 0, m2[0].size() / 2),
		l2 = sub_matrix(m2, 0, m2.size(), m2[0].size() / 2, m2[0].size());

	auto f1 = std::async([&]() {return r1 * l1; });
	auto f2 = std::async([&]() {return r1 * l2; });
	auto f3 = std::async([&]() {return r2 * l1; });
	auto f4 = std::async([&]() {return r2 * l2; });

	std::vector<matrix> mx = { f1.get(), f2.get(), f3.get(), f4.get() };

	int splits = 2;
#elif !STATIC_4_T

	int splits = sqrt(threads);

	std::vector<matrix> r, l;

	//split matrices
	for (size_t i = 0; i < splits; i++)
	{
		r.push_back(sub_matrix(m1, i * (m1.size() / splits), (i + 1) * (m1.size() / splits), 0, m1[0].size()));
		l.push_back(sub_matrix(m2, 0, m2.size(), i * (m2[0].size() / splits), (i + 1) * (m2[0].size() / splits)));
	}

	std::vector<std::future<matrix>> fs;
	//spawn threads via std::async, mainly used because it returns a future wich offers a syncronasation point
	for (size_t i = 0; i < splits; i++)
	{
		for (size_t j = 0; j < splits; j++)
		{
			fs.push_back(std::async([&, i, j]() {return r.at(i) * l.at(j); }));
		}
	}

	std::vector<matrix> mx;
	//wait for all thread to finish
	//wait is made so that the matrices are also in correct order
	for (size_t i = 0; i < threads; i++)
	{
		mx.push_back(fs[i].get());
	}
#endif
	auto res = merge_matrices(mx, splits);

	return res;
}

void init_random_sqare_matrix(matrix& m, unsigned int size) {
	for (size_t i = 0; i < size; i++)
	{
		m.push_back(std::vector<int>());
		for (size_t j = 0; j < size; j++)
		{
			m[i].push_back(rand() % 2);
		}
	}
}

void printm_in_file(const char* name, matrix& m) {
	std::fstream f;

	f.open(name, std::ios::out);

	for (size_t i = 0; i < m.size(); i++)
	{
		for (size_t j = 0; j < m[0].size(); j++)
		{
			f << "[" << m[i][j] << "]";
		}
		f << std::endl;
	}
}

int main() {
	matrix m1, m2;
	init_random_sqare_matrix(m1, 3000);
	init_random_sqare_matrix(m2, 3000);

	auto startTime = std::chrono::system_clock::now();

	auto r = multMultiThread(m1, m2, 1);

	auto endTime = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsedSecs = endTime - startTime;

	std::cout << "calculation done in: " << elapsedSecs.count() << "seconds" << std::endl;

	printm_in_file("result.txt", r);

	std::cout << "result written in file";

	std::cin.get();
}