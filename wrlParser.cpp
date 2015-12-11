#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>

long int BYTES_READ = 1000;

class Vertices {
public:
	float * array;
	int length;
	int index;
	void createArray(void) {
		length = 1000;
		index = 0;
		array = (float *) malloc(length*sizeof(float));
	}

	void add(float elem) {
		index++;
		if (index != length) {
			array[index-1] = elem;
		} else {
			float * temp;
			length = length + 1000;
			temp = (float *) malloc(length * sizeof(float));
			for (int i = 0; i<index-1; i++) {
				temp[i] = array[i];
			}
			
			free(array);
			array = temp;
		}
	}
};

void addVertex (Vertices * Verts, regmatch_t * v, char * str, float * out, int i){
	int length = v[0].rm_eo - v[0].rm_so;
	char * newStr = (char *) malloc(sizeof(char)*(length+1));
	strncpy(newStr, str+v[0].rm_so, length);
	newStr[length] = '\0';
	out[i%3] = strtof(newStr, NULL);
	Verts->add(out[i%3]);
	// fprintf(stdout, "Found: %f\n", out[*i%3]);
	free(newStr);
}

void pushFace (Vertices * Verts, regmatch_t * v, char * str, float * face, int i) {
	int length = v[0].rm_eo - v[0].rm_so;
	char * newStr = (char *) malloc(sizeof(char)*(length+1));
	strncpy(newStr, str+v[0].rm_so, length);
	newStr[length] = '\0';
	int index = strtof(newStr, NULL);
	fprintf(stdout, "%i\n", index);
	index = index*3;
	face[(i%3)*3] = Verts->array[index];
	face[(i%3)*3+1] = Verts->array[index+1];
	face[(i%3)*3+2] = Verts->array[index+2];
	fprintf(stderr, "f1 %f %f %f\n", face[(i%3)*3], face[(i%3)*3+1], face[(i%3)*3+2]);
	fprintf(stdout, "V %f %f %f\n", Verts->array[index], Verts->array[index+1], Verts->array[index+2]);
	free(newStr);
	i+=1;
}


bool checkForEnd(char * buffer) {
	if (strstr(buffer, "]")) {
		fprintf(stdout, "DONE\n");
		return true;
	}
	return false;
}

bool checkReg (regex_t * reg, char * buffer) {
	int reti;
	reti = regexec(reg, buffer, 0, NULL, 0);
	if (!reti) {
		return true;
	}
	return false;
}

int strIndex (char * buff, char * match, int length) {
	int m_num = 0;

	for (int i = 0; i<BYTES_READ; i++) {
		if (m_num == length) {
			return i;
		}
		if (buff[i] == match[m_num]) {
			m_num++;
		}
		else if (buff[i] != match[m_num]) {
			m_num = 0;
		}
	}
	return BYTES_READ;
}

int parse(char * path) {
	FILE *file;
	file = fopen(path, "r");
	if (file == NULL) {
		fputs("Error Reading File", stderr);
		return 1;
	}

	Vertices Verts;
	char * buff;
	char * tempStr;
	size_t res;
	const char coor[] = "Coordinate3";
	const char face[] = "IndexedFaceSet";
	

	bool reading_faces = false;
	bool done_faces = false;
	bool first_faces = true;
	bool reading_verts = false;
	bool done_verts = false;
	bool first_verts = true;
	regex_t regF;
	regex_t regV;
	regex_t regCoor;
	regex_t regInd;
	int ret;
	char * vertInd;
	char * facInd;
	regmatch_t v[3];
	regmatch_t f[4];
	int offset;
	int length;
	int i = 0;
	float out[3];
	float faceList[12];

	ret = regcomp(&regF, "IndexedFaceSet", 0);
	if (ret) {
		fprintf(stderr, "Could not compile regex for faces\n");
		exit(1);
	}

	ret = regcomp(&regV, coor, 0);
	if (ret) {
		fprintf(stderr, "Could not compile regex for vertices\n");
		exit(1);
	}

	ret = regcomp(&regCoor, " [-]?[0-9].[0-9]+", REG_EXTENDED);
	if (ret) {
		fprintf(stderr, "Could not compile regex for Coordinates\n");
		exit(1);
	}

	ret = regcomp(&regInd, " [0-9]*", REG_EXTENDED);
	if (ret) {
		fprintf(stderr, "Could not compile regex for Indeces\n");
		exit(1);
	}

	buff = (char *) malloc(sizeof(char)*BYTES_READ);
	if (buff == NULL) {fputs ("Memory error",stderr); exit (2);}

	Verts.createArray();
	do {
		res = fread(buff, 1, BYTES_READ, file);

		if (!reading_faces && !done_faces) {
			reading_faces = checkReg(&regF, buff);
			if (reading_faces) {
				facInd = strstr(buff, face);
				fprintf(stdout, "Faces Start at: %li\n", facInd-buff);
			}
		}

		if (!reading_verts && !done_verts) {
			reading_verts = checkReg(&regV, buff);
			if (reading_verts) {
				vertInd = strstr(buff, coor); 
				fprintf(stdout, "Vertices Start at: %li\n", vertInd-buff);
			}
		}

		//Match vertices in file, add vertices to Vertices. 
		if (reading_verts && !done_verts) {
			if (first_verts) {
				offset = 0;
				do {
					ret = regexec(&regCoor, vertInd+offset, 1, v, 0);
					if (!ret) {
						addVertex(&Verts, v, vertInd+offset, out, i);
						offset += v[0].rm_eo;
						i++;
					}
				} while (ret == 0);
				tempStr = (char *) malloc(sizeof(char)*21);
				strncpy(tempStr, buff+BYTES_READ-10, 10);
				tempStr[20] = '\0';
				first_verts = false;
			} else {
				if (checkForEnd(buff)) {
					char * tempInd;
					tempInd = strstr(buff, "]");
					length = tempInd-buff;
					char * temp2 = (char *) malloc(sizeof(char)*length+1);
					strncpy(temp2, buff, length);
					temp2[length] = '\0';
					offset = 0;
					do {
						ret = regexec(&regCoor, temp2+offset, 1, v, 0);
						if (!ret) {
							addVertex(&Verts, v, temp2+offset, out, i);
							offset += v[0].rm_eo;
							i++;
						}
					} while (ret == 0);
					free(tempStr);
					free(temp2);
					done_verts = true;
				} else {
					strncpy(tempStr+10, buff, 10);
					if (tempStr[0] != ' ') {
						ret = regexec(&regCoor, tempStr, 1, v, 0);
						if(!ret) {
							addVertex(&Verts, v, tempStr, out, i);
							i++;
						}
					} else if (tempStr[10] != ' '){
						ret = regexec(&regCoor, tempStr+9, 1, v, 0);
						if(!ret) {
							addVertex(&Verts, v, tempStr+9, out, i);
							i++;
						}
					}
					offset = 0;
					do {
						ret = regexec(&regCoor, buff+offset, 1, v, 0);
						if (!ret) {
							addVertex(&Verts, v, buff+offset, out, i);
							offset += v[0].rm_eo;
							i++;
						}
					} while (ret == 0);
					strncpy(tempStr, buff+BYTES_READ-10, 10);
				}
			}
		}

				//Match faces in file, create face array from vertices, update shared
		if (reading_faces && !done_faces) {
			//( -?[0-9]*,){4}
			if (first_faces) {
				offset = 0;
				i = 0;
				do {
					ret = regexec(&regCoor, facInd+offset, 1, v, 0);
					if (!ret) {
						pushFace(&Verts, v, facInd+offset, faceList, i);
						offset += v[0].rm_eo;
						i++;
					}
				} while (ret == 0);
				tempStr = (char *) malloc(sizeof(char)*11);
				strncpy(tempStr, buff+BYTES_READ-5, 5);
				tempStr[10] = '\0';
				first_faces = false;
			} else {
				if (checkForEnd(buff)) {
					char * tempInd;
					tempInd = strstr(buff, "]");
					length = tempInd-buff;
					char * temp2 = (char *) malloc(sizeof(char)*length+1);
					strncpy(temp2, buff, length);
					temp2[length] = '\0';
					offset = 0;
					do {
						ret = regexec(&regCoor, temp2+offset, 1, v, 0);
						if (!ret) {
							pushFace(&Verts, v, temp2+offset, faceList, i);
							offset += v[0].rm_eo;
							i++;
						}
					} while (ret == 0);
					free(tempStr);
					free(temp2);
					done_faces = true;
				} else {
					strncpy(tempStr+10, buff, 10);
					if (tempStr[0] != ' ') {
						ret = regexec(&regCoor, tempStr, 1, v, 0);
						if(!ret) {
							pushFace(&Verts, v, tempStr, faceList, i);
							i++;
						}
					} else if (tempStr[10] != ' '){
						ret = regexec(&regCoor, tempStr+9, 1, v, 0);
						if(!ret) {
							pushFace(&Verts, v, tempStr+9, faceList, i);
							i++;
						}
					}
					offset = 0;
					do {
						ret = regexec(&regCoor, buff+offset, 1, v, 0);
						if (!ret) {
							pushFace(&Verts, v, buff+offset, faceList, i);
							offset += v[0].rm_eo;
							i++;
						}
					} while (ret == 0);
					strncpy(tempStr, buff+BYTES_READ-5, 5);
				}
			}			
		}
	} while (res == BYTES_READ);
	fprintf(stdout, "Verts length: %i\n", Verts.index);
	fprintf(stdout, "%f %f %f\n", Verts.array[1059*3], Verts.array[1059*3+1],  Verts.array[1059*3+2]);
	fprintf(stdout, "Last Vert: %f %f %f\n", Verts.array[Verts.index-3], Verts.array[Verts.index-2],Verts.array[Verts.index-1]);
	fprintf(stdout, "Second to Last: %f %f %f\n", Verts.array[Verts.index-6], Verts.array[Verts.index-5], Verts.array[Verts.index-4]);
	fclose(file);
	free (buff);
	return 0;
}

int main(int argc, char **argv){

	if (argc < 2) {
		fputs("Too few arguments\n", stderr);
		return 1;
	}

	
	int out = parse(argv[argc-1]);
	if (out != 0) {
		fprintf(stderr, "Error parsing: %i", out);
		return 1;
	} 
	return 0;
}