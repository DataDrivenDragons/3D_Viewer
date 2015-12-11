// #include <regex.h>

class Vertices {
public:
	float * array;
	int length;
	int index;
	void createArray(void);
	void add(float elem);
};

void addVertex (Vertices * Verts, regmatch_t * v, char * str, float * out, int i);
void pushFace (Vertices * Verts, regmatch_t * v, char * str, float * face, int i, Queue * queue);
bool checkForEnd(char * buffer);
bool checkReg (regex_t * reg, char * buffer);
int strIndex (char * buff, char * match, int length);
int parseFunc(char * path, Queue * queue);
