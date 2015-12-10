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

bool checkForEnd(char * buffer) {
	if (strstr(buffer, "]")) {
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
	char * temp;
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

	ret = regcomp(&regCoor, "[-]?[0-9].[0-9]+", REG_EXTENDED);
	if (ret) {
		fprintf(stderr, "Could not compile regex for Coordinates\n");
		exit(1);
	}

	ret = regcomp(&regInd, "-?[0-9]*", REG_EXTENDED);
	if (ret) {
		fprintf(stderr, "Could not compile regex for Indeces\n");
		exit(1);
	}

	buff = (char *) malloc(sizeof(char)*BYTES_READ);
	if (buff == NULL) {fputs ("Memory error",stderr); exit (2);}

	Verts.createArray();
	do {
		res = fread(buff, 1, BYTES_READ, file);
		// fprintf(stdout, "Bytes Read: %li\n", res); 
		// fprintf(stdout, "Buffer: %s\n", buff);

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

		//Match faces in file, create face array from vertices, update shared
		if (reading_faces) {
			//( -?[0-9]*,){4}
			if (first_faces) {
				first_faces = false;
			} else {

			}			
		}

		//Match vertices in file, add vertices to Vertices. 
		if (reading_verts) {
			//( -?[0-9.]*){3}
			if (first_verts) {
				offset = 0;
				fprintf(stdout, "%s\n", vertInd);
				do {
					ret = regexec(&regCoor, vertInd+offset, 1, v, 0);
					if (!ret) {
						length = v[0].rm_eo - v[0].rm_so;
						char * newStr = (char *) malloc(sizeof(char)*(length+1));
						strncpy(newStr, vertInd+v[0].rm_so+offset, length);
						newStr[length] = '\0';
						offset += v[0].rm_eo;
						i+=1;
						fprintf(stdout, "String: %s Length: %i\n", newStr, length);
						out[i%3] = strtof(newStr, NULL);
						if ((i+1)%3 == 0) {
							fprintf(stdout, "%f %f %f\n", out[0], out[1], out[2]);
						}
						free(newStr);
					}
				} while (ret == 0);
				first_verts = false;
			} else {
				if (checkForEnd(buff)) {
					done_verts = true;
					continue;
				}
				offset = 0;
				do {
					ret = regexec(&regCoor, buff+offset, 1, v, 0);
					if (!ret) {
						length = v[0].rm_eo - v[0].rm_so;
						char * newStr = (char *) malloc(sizeof(char)*(length+1));
						strncpy(newStr, buff+v[0].rm_so+offset, length);
						newStr[length] = '\0';
						offset += v[0].rm_eo;
						i+=1;
						fprintf(stdout, "String: %s Length: %i\n", newStr, length);
						out[i%3] = strtof(newStr, NULL);
						if ((i+1)%3 == 0) {
							fprintf(stdout, "%f %f %f\n", out[0], out[1], out[2]);
						}
						free(newStr);
					}
				} while (ret == 0);
			}


		}

	} while (res == BYTES_READ);

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