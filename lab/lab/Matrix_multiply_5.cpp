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
#include "../../Downloads/lab/everest.hpp"
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
		((Link*)a)->on_t(*(everest_task*)t);
	}

	Link(templet::engine& e, templet::everest_engine& te_base) :Link() {
		Link::engines(e, te_base);
	}

	Link() :templet::actor(true),
		out(this, &on_out_adapter),
		stop(this, &on_stop_adapter),
		t(this, &on_t_adapter)
	{
		/*$TET$Link$Link*/
		t.app_id("646e1c50100000130083a3e7");
		num_of_ready_columns = 0;
		/*$TET$*/
	}

	void engines(templet::engine& e, templet::everest_engine& te_base) {
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
		inner = m;
		std::cout << i << "actor receive" << m.j << std::endl;
		int j = m.j;

		json in;
		in["name"] = "lab_vostokin";
		in["inputs"]["x1"] = A[i][0];
		in["inputs"]["x2"] = A[i][1];
		in["inputs"]["x3"] = A[i][2];

		in["inputs"]["x4"] = B[0][j];
		in["inputs"]["x5"] = B[1][j];
		in["inputs"]["x6"] = B[2][j];

		if (t.submit(in)) std::cout << "task submit succeeded" << std::endl;
		else std::cout << "task submit failed" << std::endl;

		std::cout << i << "actor send" << m.j << std::endl;
		next->in(m); m.send();
		if (++num_of_ready_columns == N) stop.send();

		/*$TET$*/
	}

	inline void on_out(column& m) {
		/*$TET$Link$out*/
		/*$TET$*/
	}
	inline void on_t(everest_task& t) {
		/*$TET$mediator$t*/
		json out = t.result();
		C[i][inner.j] = out["y"];

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
	everest_task t;

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
	templet::everest_engine teng("username", "password");

	Stopper stopper(eng);
	Link l[N];

	for (int i = 0; i < N; i++) {
		l[i].engines(eng, teng); stopper.in(l[i].stop);
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

// ������ ���������: CTRL+F5 ��� ���� "�������" > "������ ��� �������"
// ������� ���������: F5 ��� ���� "�������" > "��������� �������"

// ������ �� ������ ������ 
//   1. � ���� ������������ ������� ����� ��������� ����� � ��������� ���.
//   2. � ���� Team Explorer ����� ������������ � ������� ���������� ��������.
//   3. � ���� "�������� ������" ����� ������������� �������� ������ ������ � ������ ���������.
//   4. � ���� "������ ������" ����� ������������� ������.
//   5. ��������������� �������� ������ ���� "������" > "�������� ����� �������", ����� ������� ����� ����, ��� "������" > "�������� ������������ �������", ����� �������� � ������ ������������ ����� ����.
//   6. ����� ����� ������� ���� ������ �����, �������� ������ ���� "����" > "�������" > "������" � �������� SLN-����.
