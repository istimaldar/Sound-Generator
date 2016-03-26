#include <iostream.h>
#include <fstream.h>
#include <math.h>
#include <string.h>
#include <dos.h>
#include <conio.h> 

static const short BUFFER_SIZE = 100;
static const long BASE = 1193000;
int tempo;

int decryptTheParameter(char* parameter)
{
	if (strcmp(parameter, "GES-DUR") == 0 || strcmp(parameter, "EES-MOLL") == 0) return -6;
	if (strcmp(parameter, "DES-DUR") == 0 || strcmp(parameter, "B-MOLL") == 0) return -5;
	if (strcmp(parameter, "AES-DUR") == 0 || strcmp(parameter, "F-MOLL") == 0) return -4;
	if (strcmp(parameter, "EES-DUR") == 0 || strcmp(parameter, "C-MOLL") == 0) return -3;
	if (strcmp(parameter, "B-DUR") == 0 || strcmp(parameter, "G-MOLL") == 0) return -2;
	if (strcmp(parameter, "F-DUR") == 0 || strcmp(parameter, "D-MOLL") == 0) return -1;
	if (strcmp(parameter, "C-DUR") == 0 || strcmp(parameter, "A-MOLL") == 0) return 0;
	if (strcmp(parameter, "G-DUR") == 0 || strcmp(parameter, "E-MOLL") == 0) return 1;
	if (strcmp(parameter, "D-DUR") == 0 || strcmp(parameter, "H-MOLL") == 0) return 2;
	if (strcmp(parameter, "A-DUR") == 0 || strcmp(parameter, "FIS-MOLL") == 0) return 3;
	if (strcmp(parameter, "E-DUR") == 0 || strcmp(parameter, "CIS-MOLL") == 0) return 4;
	if (strcmp(parameter, "H-DUR") == 0 || strcmp(parameter, "GIS-MOLL") == 0) return 5;
	if (strcmp(parameter, "FIS-DUR") == 0 || strcmp(parameter, "DIS-MOLL") == 0) return 6;
	return 0;
}

template <class T>
class elementOfQueue
{
public:
	elementOfQueue<T> *next;
	elementOfQueue<T> *previous;
	T data;
	elementOfQueue() {}
};

template <class T>
class queue
{
	elementOfQueue<T> *_back;
	elementOfQueue<T> *_front;
public:
	queue()
	{
		_back = NULL;
		_front = NULL;
	}
	T front() { return _front->data; }
	T back() { return _back->data; }
	void push(T& data)
	{
		elementOfQueue<T> *temp = new elementOfQueue<T>();
		temp->data = data;
		temp->next = NULL;
		temp->previous = _back;
		if (_back != NULL) _back->next = temp;
		_back = temp;
		if (_front == NULL) _front = temp;
	}
	void pop()
	{
		if (_front == NULL) return;
		if (_front == _back) _back = NULL;
		_front = _front->next;
		if (_front != NULL)_front->previous = NULL;
	}
	int empty()
	{
		if (_back == NULL&&_front == NULL) return 1;
		return 0;
	}
};

class note
{
public:
	int frequency;
	int duration;
	int pause;
	note(int fr, int dur, int pa) : frequency(fr), duration(dur), pause(pa) {};
};

int translate(char* string, int tonality)
{
	int degree;
	switch (string[0])
	{
	case 'P':
		return 0;
		break;
	case 'C':
		degree = 1;
		if (tonality >= 2) degree++;
		if (tonality <= -6) degree--;
		break;
	case 'D':
		degree = 3;
		if (tonality >= 4) degree++;
		if (tonality <= -4) degree--;
		break;
	case 'E':
		degree = 5;
		if (tonality >= 6) degree++;
		if (tonality <= -2) degree--;
		break;
	case 'F':
		degree = 6;
		if (tonality >= 1) degree++;
		break;
	case 'G':
		degree = 8;
		if (tonality >= 3) degree++;
		if (tonality <= -5) degree--;
		break;
	case 'A':
		degree = 10;
		if (tonality >= 5) degree++;
		if (tonality <= -3) degree--;
		break;
	case 'B':
		degree = 11;
		break;
	case 'H':
		degree = 12;
		if (tonality <= -1) degree--;
		break;
	}
	for (int i = 1; i < strlen(string); i += 2)
	{
		if (string[i] == 'I' && string[i+1] == 'S') degree++;
		if (string[i] == 'E' && string[i + 1] == 'S') {
			degree--;
			if (!degree) degree--;
		}
	}
	return degree;
}

int getFrequency(int octave, int degree)
{
	if (!degree) return 0;
	else if (degree < 0)
	{
		degree++;
		while (degree<0)
		{
			octave--;
			degree += 12;
		}
	}
	else if (degree > 12)
	{
		while (degree>12)
		{
			octave++;
			degree -= 12;
		}
	}
	double frequency = 440;
	frequency *= pow(2, octave - 4);
	frequency *= pow(2,  (degree - 10) / 12.);
	return (int)(frequency);
}

void loadFromFile(queue<note*>& nqueue,char* fileName)
{
	char buffer[BUFFER_SIZE];
	char parameter[BUFFER_SIZE];
	note *temp;
	int duration, numinator, denominator, octave, frequency, degree, connected,tonality;
	ifstream reader(fileName);
	reader >> tempo >> parameter;
	tonality = decryptTheParameter(parameter);
	while (!reader.eof())
	{
		reader >> buffer >> numinator >> denominator  >> connected;
		octave = buffer[strlen(buffer) - 1] - '0';
		buffer[strlen(buffer) - 1] = '\0';
		degree = translate(buffer,tonality);
		frequency = getFrequency(octave, degree);
		duration = numinator * 240000 / tempo / denominator;
		if (!frequency)	nqueue.back()->duration += duration;
		else {
			 temp = new note(frequency, duration, 5 + (1-connected) * 5);
			nqueue.push(temp);
		}
	}
	reader.close();
}

void play(queue<note*>& nqueue)
{
	note *temp;
	while (!nqueue.empty())
	{
		temp = nqueue.front();
		nqueue.pop();
		int kd;
		outp(0x43, 0xB6);
		kd = BASE / temp->frequency;
		outp(0x42, kd % 256); //Low
		kd /= 256;         //Do smth for pause
		outp(0x42, kd);     //High
		outp(0x61, inp(0x61) | 3);    //turn ON
		delay(temp->duration);
		outp(0x61, inp(0x61) & 0xFC);
		delay(temp->pause);
		delete temp;
	}
}

int main()
{
	queue<note*> * nqueue = new queue<note*>();
	cout << "Enter melody number\n1. Twinkle, twinkle little star\n2. Seven Nation Army\n3. Bad Apple\n";
	int number;
	cin >> number;
	char *fileName = new char[BUFFER_SIZE];
	switch (number)
	{
	case 1:
		fileName = "Twinkle.txt";
		break;
	case 2:
		fileName = "Seven.txt";
		break;
	case 3:
		fileName = "Bad.txt";
		break;
	}
	loadFromFile(*nqueue,fileName);
	play(*nqueue);
	delete fileName;
	return 0;
}

