/*$TET$$header*/
/*--------------------------------------------------------------------------*/
/*  Copyright 2023 Sergei Vostokin                                          */
/*                                                                          */
/*  Licensed under the Apache License, Version 2.0 (the "License");         */
/*  you may not use this file except in compliance with the License.        */
/*  You may obtain a copy of the License at                                 */
/*                                                                          */
/*  http://www.apache.org/licenses/LICENSE-2.0                              */
/*                                                                          */
/*  Unless required by applicable law or agreed to in writing, software     */
/*  distributed under the License is distributed on an "AS IS" BASIS,       */
/*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*/
/*  See the License for the specific language governing permissions and     */
/*  limitations under the License.                                          */
/*--------------------------------------------------------------------------*/


#pragma once

#include "../../Downloads/lab/templet.hpp"
#include <iostream>

using namespace templet;
using namespace std;

const int N = 3;

double A[N][N] = { {1.0,2.0,3.0},
				   {1.0,2.0,3.0},
				   {1.0,2.0,3.0} };

double B[N][N] = { {1.0,1.0,1.0},
				   {2.0,2.0,2.0},
				   {3.0,3.0,3.0} };

double C[N][N] = { 0.0 };

class column : public templet::message {
public:
	column(templet::actor* a = 0, templet::message_adaptor ma = 0) :templet::message(a, ma) {}
	int j; // column index
};

class Multiplytask : protected templet::base_task {
public:
	Multiplytask(templet::actor* a, templet::task_adaptor ta) :base_task(a, ta) {}
	void engine(templet::base_engine& te) { templet::base_task::engine(te); }
	void submit_k(int r, int c) { i = r; j = c; submit(); }
private:
	void run() override { for (int k = 0; k < N; k++) C[i][j] += A[i][k] * B[k][j]; }
	int i;
	int j;
};

/*$TET$*/

#pragma templet !Link(in?column,out!column,stop!message)

struct Link :public templet::actor {
	static void on_in_adapter(templet::actor* a, templet::message* m) {
		((Link*)a)->on_in(*(column*)m);
	}
	static void on_out_adapter(templet::actor* a, templet::message* m) {
		((Link*)a)->on_out(*(column*)m);
	}
	static void on_stop_adapter(templet::actor* a, templet::message* m) {
		((Link*)a)->on_stop(*(message*)m);
	}
	static void on_t_adapter(templet::actor* a, templet::task* t) {
		((Link*)a)->on_t(*(Multiplytask*)t);
	}

	Link(templet::engine& e, templet::base_engine& te_base) :Link() {
		Link::engines(e,te_base);
	}

	Link() :templet::actor(true),
		out(this, &on_out_adapter),
		stop(this, &on_stop_adapter),
		t(this, &on_t_adapter)
	{
		/*$TET$Link$Link*/
		num_of_ready_columns = 0;
		/*$TET$*/
	}

	void engines(templet::engine& e, templet::base_engine& te_base) {
		templet::actor::engine(e);
		t.engine(te_base);
		/*$TET$Link$engines*/
		/*$TET$*/
	}

	void start() {
		/*$TET$Link$start*/
		on_in(out);
		/*$TET$*/
	}

	inline void on_in(column& m) {
		/*$TET$Link$in*/
		
		if (num_of_ready_columns != 0 && &m == &out) return;
		
		std::cout << i << "actor receive" << m.j << std::endl;
		int j = m.j;
		
		t.submit_k(i, j);
		std::cout << i << "actor send" << m.j << std::endl;
		next->in(m); m.send();
		if (++num_of_ready_columns == N) stop.send();
		
		/*$TET$*/
	}

	inline void on_out(column& m) {
		/*$TET$Link$out*/
		/*$TET$*/
	}
	inline void on_t(Multiplytask& t) {
		/*$TET$mediator$t*/
		//t.delay(DELAY);
		
		/*$TET$*/
	}

	inline void on_stop(message& m) {
		/*$TET$Link$stop*/
		/*$TET$*/
	}

	void in(column& m) { m.bind(this, &on_in_adapter); }
	column out;
	column inner;
	message stop;
	Multiplytask t;

	/*$TET$Link$$footer*/
	int i; // row index
	int num_of_ready_columns;
	Link* next;

	/*$TET$*/
};

#pragma templet !Stopper(in?message)

struct Stopper :public templet::actor {
	static void on_in_adapter(templet::actor* a, templet::message* m) {
		((Stopper*)a)->on_in(*(message*)m);
	}

	Stopper(templet::engine& e) :Stopper() {
		Stopper::engines(e);
	}

	Stopper() :templet::actor(true)
	{
		/*$TET$Stopper$Stopper*/
		num_of_ready_rows = 0;
		/*$TET$*/
	}

	void engines(templet::engine& e) {
		templet::actor::engine(e);
		/*$TET$Stopper$engines*/
		/*$TET$*/
	}

	void start() {
		/*$TET$Stopper$start*/
		/*$TET$*/
	}

	inline void on_in(message& m) {
		/*$TET$Stopper$in*/
		if (++num_of_ready_rows == N) stop();
		/*$TET$*/
	}

	void in(message& m) { m.bind(this, &on_in_adapter); }

	/*$TET$Stopper$$footer*/
	int num_of_ready_rows;
	/*$TET$*/
};

/*$TET$$footer*/
/*$TET$*/

int main()
{
	engine eng;
	templet::base_engine teng;

	Stopper stopper(eng);
	Link l[N];

	for (int i = 0; i < N; i++) {
		l[i].engines(eng,teng); stopper.in(l[i].stop);
		l[i].next = &l[(i + 1) % N];

		l[i].i = i; l[i].out.j = i;
	}

	eng.start();
	teng.run();

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			std::cout << "  C[" << i << "," << j << "] = " << C[i][j];
		}
		std::cout << endl;
	}

	std::cout << endl;

	std::cout << (eng.stopped() ? "Success!!!" : "Failure...");
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
