int Integer(const char *input){
    int power = 0;
    int output = 0;
    int condition = 0;
    int i = strlen(input) - 1;
    bool sign = input[0] == '-' ? true:false; //assign true if negative, or false if positive
    if(sign){
        condition = 1;
    }
    while(i >= condition){  // then do basic conversion from string to integer
        output += (int)(input[i--] - 48) * pow(10,power++);
    }
    if(sign){
        return -output;
    }
    return output;
}

// this function checks if a given string is valid or not, not very efficient, it just checks if the string is longer than 5 chars.
// then checks if the first 4 chars are string representation of digits 
bool isValid(char *s){
    if(strlen(s) <= 5){
        return false;
    }
    for(int i = 0; i < 4; i++){
        if( s[i] < '0' || s[i] > '9'){
            return false;
        }
    }
    return true;
}
 
// this function extracts a substring from a given string (s), based on a given delimiter, returns a sting and an index t continue from s
void substring(char *s,char delimiter,int *beginningIndex, char *output){
    int i = *beginningIndex;
    int j = 0;
    int n = strlen(s);
    while(s[i] != delimiter && i < n){
        output[j++] = s[i++];
    }
    *beginningIndex = i + 1; //because at i we have that delimiter
    output[j] = '\0';
}

// this function parses a string and returns its equivalent record
// it follows a very straightforward approach
Record parse(char *s){
    char temp[64];
    int stringIndex = 0;
    Record data;
    substring(s,'$',&stringIndex,temp);
    data.key = Integer(temp);
    memset(temp, 0, sizeof(temp));

    substring(s,'$',&stringIndex,temp);
    data.eraser = temp[0] == '1';
    memset(temp, 0, sizeof(temp));

   substring(s,'$',&stringIndex,temp);
    sprintf(data.firstName,"%s",temp);
    memset(temp, 0, sizeof(temp));



    substring(s,'$',&stringIndex,temp);
    sprintf(data.lastName,"%s",temp);
    memset(temp, 0, sizeof(temp));

    substring(s,'\0',&stringIndex,temp);
    sprintf(data.school,"%s",temp);
    memset(temp, 0, sizeof(temp));

    return data;
}

// slices a string and returns a substring form index beg to end
char *sliceString(char *input, int beg, int End){
    int size = (End - beg);
    char *output = (char *)malloc(sizeof(char) * size);

    for(int i = 0; beg < End; i++){
        output[i]= input[beg++];
    }
    output[size ] = '\0';
    return output;
}

// this function converts a record into a string to be written in the block (buffer) then the file
char *stringify(Record parcel,const char delimiter){   // we convert our record into a string, separated by some given delimiter (fixed in the whole file)
    char *output = (char *)malloc(sizeof(char) * 144);
    sprintf(output,"%.4d%c%d%c%s%c%s%c%s|",parcel.key,delimiter,parcel.eraser,delimiter,parcel.firstName,delimiter,parcel.lastName,delimiter,parcel.school);
    return output;
}

// FROM NOW ON, WE WILL HAVE ONLY FILE OP. FUNCTIONS

// opens a tovs file
int openTOVSFile(TOVS_file *file, const char *fileName,const char mode){
    if(toupper(mode) == 'N'){           // if new file, we create a file, and we set all the fields of the header to 0
        file->file = fopen(fileName,"wb+");
        file->header.nb_Blocks = 0;
        file->header.nb_chars = 0;
        file->header.nb_del_chars = 0;
        file->header.nb_records = 0;
        return 1;
    } else if(toupper(mode) == 'O'){    // if old, we open the file
        file->file = fopen(fileName,"rb+");
        if(file->file == NULL){
            return -1;          // not found
        }
        if(fread(&(file->header),sizeof(Header),1,file->file) != 1){
            return -2;          // couldn't read header (something went wrong)
        }
        return 1;   // done 
    }
    return -2;          // something went wrong, mode isn't valid
}

// closes a tovs file
int closeTOVSFile(TOVS_file file){
    rewind(file.file);
    if(fwrite(&(file.header),sizeof(Header),1,file.file) != 1){
        return -1;  // couldn't write the header
    }
    fclose(file.file);  // closing the file
    return 1;   // done
}

// gets the value of a given field from the header
int getTOVSHeader(TOVS_file file,int field){
    switch (field)
    {
    case 1:
        return file.header.nb_Blocks;           // number of blocks
    case 2:
        return file.header.nb_chars;            // number of written chars.
    case 3:
        return file.header.nb_del_chars;        // number of removed chars (logical deletion)
    case 4:
        return file.header.nb_records;          // number of records
    }   
    return -1;  //invalid field
}


// updates the value of a given filed, the idea is a little bit similar to the getters ad setters in other languages like JS
int setTOVSHeader(TOVS_file *file,int field, int value){
    switch (field)
    {
    case 1:
        file->header.nb_Blocks = value;     // update number of blocks
        break;
    case 2:
        file->header.nb_chars = value;      // update number of written chars.
        break;
    case 3:
        file->header.nb_del_chars = value;  // update the number of removed chars
        break;
    case 4:
        file->header.nb_records = value;        // update the number of records
        break;
    default:
        return -1;      // not valid field
    }
    return 0;
}

// display the header of a tovs file
void displayTOVSHeader(TOVS_file file, char *fileName){
    printf("File type:  TOVS\n");
    printf("File name: %s\n", fileName);
    printf("Number of Blocks:  %d\n", getTOVSHeader(file,1));
    printf("Number of characters:  %d\n", getTOVSHeader(file,2));
    printf("Number of deleted characters:  %d\n", getTOVSHeader(file,3));
    printf("Number of records:  %d\n", getTOVSHeader(file,4));
}


// bulk loading function of the tovs file
int bulkLoadingTOVS(TOVS_file *file, TOVS_Block *data,int dataLen,int nbOfRecords, int nbOfChars){
    if(file->file == NULL){     // checking if the file is open or not
        return -1;
    }
    // we insert an array of Blocks  all at once
    fseek(file->file,sizeof(Header),SEEK_SET);
    if(fwrite(data,sizeof(TOVS_Block),dataLen,file->file) != dataLen){
        return 0;   // something went wrong if we couldn't write the data
    }
    // then after writing we update the header
    setTOVSHeader(file,1,dataLen);       
    setTOVSHeader(file,2,nbOfChars);
    setTOVSHeader(file,4,nbOfRecords);
    return 1;       // done
}

// this function loads a tovs file
int loadTOVSFile(TOVS_file *file, float loadingFactor){
    printf("Please assign ordered keys to the records to maintain the ordered structure, otherwise, the program will encounter problems doing other tasks!\n");
    int lastKey = -1;                   // we begin by a negative key to make sure all our keys are positive >= 0
    int numberOfRecords = 0;                 // we keep track of te number of records we want to insert
    Record data;                             // the following two variables are just placeholders
    char s[100];
    FILE *dataSet = fopen("dataSet.dat","rb+");         // open the data set file
    if(dataSet == NULL){
        return -1;                              // if the file is removed for some reason, we stop here
    }
    printf("how many records do you want to load: ");   // re read the keys to get ordered ones
    scanf("%d", &numberOfRecords);                   // we read the number of records the user wants to add
    getchar();                                  // remove the newline char. from the input stream
    TOVS_Block *arrayOfBlocks = (TOVS_Block *)malloc(sizeof(TOVS_Block) * (numberOfRecords / 3));  //allocate an array of blocks to load
    int j = 0;
    int block_index = 0;    // these variables will be used to keep track of the records in each block 
    int charCount = 0;       // and the number of characters t=inserted in the whole file
                    // resetting the first block before we begin using it
            (arrayOfBlocks + block_index)->maxKey = -1; // max to a very small value
            (arrayOfBlocks + block_index)->minKey = 1564894555;  // we set the min to a very big value
            (arrayOfBlocks + block_index)->nb_records = 0;
            (arrayOfBlocks + block_index)->overlaps = 0;

    for(int i = 0; i < numberOfRecords; i++){
        fread(&data,sizeof(Record),1,dataSet);
        printf("enter the key of this record: ");           // we read a record from the dataSet and we reassign a key to it to keep order
        scanf("%d", &data.key);
        while(data.key <= lastKey){
            printf("please maintain the order, enter a bigger key: ");   // we make sure the user maintains the ascending order of the keys
            scanf("%d", &data.key);     
        }
        lastKey = data.key;
        strcpy(s,stringify(data,'$'));              // convert the record to a string
        int n = strlen(s);                  // get the length of the string we want to write

        for(int k = 0; k < n; k++){         // we insert our string char. by char. to keep track of our index and prevent going out of bounds
            (arrayOfBlocks + block_index)->info[j++] = s[k];
            ++charCount;                    // keep track of the number of chars. we wrote
        }
        // we update the information of the block, maximum key, minimum key and the number of records
        (arrayOfBlocks + block_index)->maxKey = (arrayOfBlocks + block_index)->maxKey > data.key ? (arrayOfBlocks + block_index)->maxKey:data.key;
        (arrayOfBlocks + block_index)->minKey = (arrayOfBlocks + block_index)->minKey < data.key ? (arrayOfBlocks + block_index)->minKey:data.key;
        (arrayOfBlocks + block_index)->nb_records += 1;              
        if(j >= loadingFactor * MAXLEN && i + 1 < numberOfRecords){             // if we exceed the loading factor we move to the next block (next elemnet in the array)
       
                ++block_index;
                // reset the necessary fields of the next block and we start writing into it

                (arrayOfBlocks + block_index)->maxKey = -1; // max to a very small value
                (arrayOfBlocks + block_index)->minKey = 1564894555;  // we set the min to a very big value
                (arrayOfBlocks + block_index)->nb_records = 0;
                (arrayOfBlocks + block_index)->overlaps = 0;
                j = 0;
        }

    }
    // after we finish writing, we write the whole array into the file, all at once 
    bulkLoadingTOVS(file,arrayOfBlocks,block_index+1,numberOfRecords,charCount);
    return 0;
}

// writes a block
// takes the buffer and the index of the block, note that blocks are 0-indexed everywhere in our program
int writeTOVSBlock(TOVS_file *file, TOVS_Block block, int position){
    if(file->file == NULL){
        return -1;              // check if the file is open
    }
    fseek(file->file,sizeof(Header) + sizeof(TOVS_Block) * position, SEEK_SET);
    if(fwrite(&block,sizeof(TOVS_Block),1,file->file) != 1){        // write and return -0 if the op. fails
        return 0;
    }
    return 1;           // and return 1 if all is done successfully
}


//reads a block and loads it into a buffer
int readTOVSBlock(TOVS_file file,int position, TOVS_Block *data){
    if(file.file == NULL){
        return -1;          // check if the file is open
    }
    fseek(file.file,sizeof(Header) + sizeof(TOVS_Block) * position, SEEK_SET);
    if(fread(data,sizeof(TOVS_Block),1,file.file) != 1){
        return 0;               // if reading fails, return 0
    }
    return 1;           // if it succeeds, return 1
}
   
    // searching into a tovs file
bool searchTOVS(TOVS_file file, int key, int *numberOfBlock, int *beginningPosition){
    if(file.file == NULL){                  // make sure the file is open
        return false;                       
    }
    int overlapped = 0;        
    int low = 0;
    int high = getTOVSHeader(file,1) - 1;
    while(high >= low){

        int middle = (high + low) / 2;
        TOVS_Block data;
        readTOVSBlock(file,middle,&data);       // perform a regular binary search until we find the block we want
        if(data.maxKey < key) low = middle + 1;
        else if(data.minKey > key) high = middle - 1;
        else {
            if(numberOfBlock != NULL){
                *numberOfBlock = middle;        // assign that index to the argument we have if it is not null
            }   
                //  read the block, if it overlaps, we read the next block to complete the last string
             char placeholder[2 * MAXLEN];
             strcpy(placeholder,data.info);
             
             if(data.overlaps){
                    // code for overlapping
                overlapped = 0;
                readTOVSBlock(file,middle + 1,&data);
                char s[100];
                substring(data.info,'|',&overlapped,s);
                strcat(placeholder,s);
             }
            // keep slicing the string and parsing until we either find the record and return true after assigning the position to the argumaent
            // or we return false if we don't find the record
            int n = strlen(placeholder);
            char stringToParse[144];
            int i = 0;
             while (i < n){   // while we're still inbounds of the big string we read
                Record output;
                memset(stringToParse,0,sizeof(stringToParse));
                substring(placeholder,'|',&i,stringToParse);
                if(isValid(stringToParse)){
                    output = parse(stringToParse);
                    if(output.key > key){
                        if(beginningPosition != NULL){
                        *beginningPosition = i - strlen(stringToParse) - 1;
                        }
                        return false;       // not found and return the insert position, we might need it
                    }
                    if(!output.eraser && output.key == key){
                        if(beginningPosition != NULL){
                        *beginningPosition = i - strlen(stringToParse) - 1;
                        }
                        return true;        // return false
                    }
                }
            }

        }
    }
    if(numberOfBlock != NULL){
        *numberOfBlock = (high + low) / 2;      // return the middle value we had if we didn't even find the block, it will be useful when inserting
    }
    return false;
}

 // logical deletion
int deleteTOVS(TOVS_file *file,int key){            // the concept is simple, search for the record, if we find it, we update the eraser field and set it to 1
    int blockNumber = 0, offset = 0;                    // otherwise, we just quit 
    if(!searchTOVS(*file,key,&blockNumber,&offset)){
        return -1;      // not found
    }
    TOVS_Block data;
    char placeholder[2 * MAXLEN];
    char stringToParse[144];
    readTOVSBlock(*file,blockNumber,&data);
    strcpy(placeholder,data.info);
        if(data.overlaps){
            // code for overlapping
            TOVS_Block *temp = (TOVS_Block *)malloc(sizeof(TOVS_Block));
            int overlapped = 0;
            readTOVSBlock(*file,blockNumber + 1,temp);
            char s[100];
            substring(temp->info,'|',&overlapped,s);
            strcat(placeholder,s);
            free(temp);
        }
    substring(placeholder,'|',&offset,stringToParse);
    int indexForeRefill = offset - strlen(stringToParse) - 1;

    Record student = parse(stringToParse);
    student.eraser = 1;
    int n = strlen(stringToParse);
    strcpy(stringToParse,stringify(student,'$'));           // don't reduce the number of chars. to be able to renew the file later
    setTOVSHeader(file,3,getTOVSHeader(*file,3) + n);
    setTOVSHeader(file,4,getTOVSHeader(*file,4) -1);
    for(int i  = 0; i < n ; i++ ){
        placeholder[indexForeRefill++] = stringToParse[i];
    }
    offset = 0;
    indexForeRefill = 0;
    n = strlen(placeholder);        // read and slice until we find the string
    while(offset < n){
        data.info[indexForeRefill++] = placeholder[offset++];
        if(indexForeRefill == MAXLEN - 1){
            data.info[indexForeRefill] = '\0';

            writeTOVSBlock(file,data,blockNumber);
            blockNumber += 1;
            readTOVSBlock(*file,blockNumber,&data);
            indexForeRefill = 0;
        }
    }

    writeTOVSBlock(file,data,blockNumber);  // write the modified buffer
    return 1;           // quit after finishing
}

// display the overlapping information
void displayOverlappingInformationTOVS(TOVS_file file){
    TOVS_Block data;
    for(int i = 0; i < getTOVSHeader(file,1); i++){ // go through all the file
        readTOVSBlock(file,i,&data);
        if(data.overlaps){      // if the block overlaps, we complete the string
            int infoIndex = 0;
            char stringToParse[144];
            int n = strlen(data.info);
            while(infoIndex < n){ // find the last string
                substring(data.info,'|',&infoIndex,stringToParse);
            }
            infoIndex = 0;      // parse it then display it
            TOVS_Block *temp = (TOVS_Block *)malloc(sizeof(TOVS_Block));
            readTOVSBlock(file,i + 1,temp);
            char s[144];
            substring(temp->info,'|',&infoIndex,s);
            strcat(stringToParse,s);
            Record student = parse(stringToParse);
            printf("Block Number %3.4d\n",i + 1);
            if(!student.eraser){

                printf("Student with key:  %4d\n",student.key);
                printf("first Name:  %s\n",student.firstName);
                printf("Last Name:  %s\n",student.lastName);
                printf("School:  %s\n\n",student.school);
            } else {
                printf("element deleted\n\n");
            }
            free(temp);  // free memory allocated

        }
    }
}

// display a block

void displayBlockTOVS(TOVS_file file, int numberOfBlock){
    if(numberOfBlock > getTOVSHeader(file,1)){              // read the block, check for overlapping, complete the last string
        return ;                                                 // extract substrings, parse them and display  costs 2 I/O operations in the worst case (overlapping)
    }                                                                 // 1 I/O operation when ther is not overlapping
    TOVS_Block data;
    char placeHolder[2 * MAXLEN];
    readTOVSBlock(file,numberOfBlock,&data);
    if(data.nb_records == 0){
        printf("block empty!\n");
        return;
    }
    strcpy(placeHolder,data.info);
    int i = 0;
    if(data.overlaps){
        readTOVSBlock(file,numberOfBlock + 1,&data);
        char temp[50];
        substring(data.info,'|',&i,temp);
        strcat(placeHolder,temp);
        i = 0;
    }
    int n = strlen(placeHolder);        // we simply read the block, check or overlapping and handle it, then we substring and parse then display each valid string
    char stringToParse[144];
    while (i < n){
        Record output;
        memset(stringToParse,0,sizeof(stringToParse));
        substring(placeHolder,'|',&i,stringToParse);
        if(isValid(stringToParse)){
            output = parse(stringToParse);
            if(!output.eraser){
                printf("Student with key:  %.4d\n",output.key);
                printf("First Name:  %s\n",output.firstName);
                printf("Last Name:  %s\n",output.lastName);
                printf("School:  %s\n\n",output.school);
            }
        }
    }

}

// display all the file

void displayAllTOVSFile(TOVS_file file){
    char placeholder[2 * MAXLEN];               // identical to the unordered file traversal, read each block, check for overlapping
    int overlapped = 0;                             // complete the last record, extract substrings and parse then display,
    TOVS_Block temp;                                    // if half blocks overlap, it would cost 3 / 2 * N I/O operations
    for(int i = 0; i < getTOVSHeader(file,1); i++){         // could be optimized if we keep the next block after reading it in case of overlapping
        readTOVSBlock(file,i,&temp);                           // and it becomes N I/O operations always
        if(temp.nb_records > 0){
            strcpy(placeholder,temp.info);
            int infoIndex = overlapped;
            if(temp.overlaps){
                overlapped = 0;
                readTOVSBlock(file,i + 1,&temp);
                char s[100];
                memset(s,0,sizeof(s));
                substring(temp.info,'|',&overlapped,s);
                strcat(placeholder,s);
            } else {
                overlapped = 0;                 // this variable keeps the index where we begin reading in the next block, so that we make sure all strings are valid
            }

            char stringToParse[144];
            int n = strlen(placeholder);
            while (infoIndex < n){          // display each record after parsing its string
                Record student;
                memset(stringToParse,0,sizeof(stringToParse));
                substring(placeholder,'|',&infoIndex,stringToParse);
                if(isValid(stringToParse)){
                    if(!student.eraser){
                        printf("Student with key: %3.4d\n",student.key);
                        printf("First Name: %s\n",student.firstName);
                        printf("Last Name:  %s\n",student.lastName);
                        printf("School:     %s\n\n\n",student.school);
                    }
                }
                student = parse(stringToParse);


            }
        }
    }

}

// allocate an empty block in the file 
int allocateTOVSBlock(TOVS_file *file){
    TOVS_Block temp;
    temp.nb_records = 0;
    temp.overlaps = 0;
    temp.maxKey = -1;
    temp.minKey = 21354866;
    fseek(file->file,sizeof(Header) + sizeof(TOVS_Block) * getTOVSHeader(*file,1),SEEK_SET);
    if(fwrite(&temp,sizeof(Block),1,file->file) != 1){
        return 0;   // could not allocate a block
    }
    setTOVSHeader(file,1,file->header.nb_Blocks + 1);
    return 1;   // task done successfully
}

// updates the block info (maxkey and minKey in particular)
void updateBlockInfo(TOVS_Block *block,char *placeholder){
    char *s = (char *)malloc(sizeof(char) * 144);   // we just go through the block and update the minKey and the maxKey
    int pos = 0;
    int n = strlen(placeholder);
    while(pos < n){
        substring(placeholder,'|',&pos,s);
        if(isValid(s)){
            Record temp = parse(s);
            block->maxKey = block->maxKey > temp.key ? block->maxKey:temp.key;
            block->minKey = block->minKey < temp.key ? block->minKey:temp.key;
        }
    }
    free(s);
}

//writes and shifts to maintain the order and the structure of the file
int writeAndShift(TOVS_file *file,char *infoToInsert, Record data, int blockNumber, TOVS_Block currentBlock){
    bool lastBlock = 0;
    int n = strlen(infoToInsert); // get the length of the string we will be inserting
    int j = 0;
    int i = 0;
    while (i < n){
        currentBlock.info[j++] = infoToInsert[i++];         // we write the string char. by char. to avoid getting out of bounds unintentionally
        if(j == MAXLEN ){                                  // when we're at the end of the string
            currentBlock.info[MAXLEN - 1] = '\0';   // null terminate the string
            if(blockNumber == getTOVSHeader(*file,1) - 1){  // if there is no next block, allocate a new one
                allocateTOVSBlock(file);
                lastBlock = 1;
            }
            currentBlock.overlaps = 1;  // set the overlapping variable of the current block to 1
            updateBlockInfo(&currentBlock,infoToInsert);
            currentBlock.nb_records += 1;           // add a record to our block
            setTOVSHeader(file,4,getTOVSHeader(*file,4) + 1);
            writeTOVSBlock(file,currentBlock,blockNumber);    // write the block
            ++blockNumber;          // increment the block index and reset j then read a new block
            j = 0;
            readTOVSBlock(*file,blockNumber,&currentBlock);
            if(lastBlock){
                memset(currentBlock.info,0,MAXLEN);                                 // if last block we insert right away for two reasons:
                strcpy(currentBlock.info,sliceString(infoToInsert,i-1,n));              //1- if all blocks are full, we won't shift more than 144 and 144 < MAXLEN = 200
                updateBlockInfo(&currentBlock,currentBlock.info);                       //2- there is no extra data to insert so we don't need concatenate anything else
                writeTOVSBlock(file,currentBlock,blockNumber);
                return 0;
            }
            strcpy(infoToInsert,sliceString(infoToInsert,i-1,n));   // keep only the information we didn't insert in the string
            strcat(infoToInsert,currentBlock.info);
                            // concatenate with the information of the read block, and repeat until we write a block which is not 100%full
            i = 0;
            n = strlen(infoToInsert);       // get the new length of the string
        }


    }
    updateBlockInfo(&currentBlock,infoToInsert);
    writeTOVSBlock(file,currentBlock,blockNumber);      // final write of the block that was lastly treated
    return 0;
}

// inserts in a file
int insertInTOVSFile(TOVS_file *file, Record data){
    int blockNumber = 0, insertPosition = 0;   // initialize the block number and the insertion position to zero
    if(searchTOVS(*file,data.key,&blockNumber,&insertPosition)){  // perform a search for that element (Record)
        return 1;   // return 1 if found
    }

    char parsedString[144]; // if not, we declare a string to hold the stringified record
    strcpy(parsedString,stringify(data,'$'));  // we stringify the given record, and we copy it into parsedString

    TOVS_Block block;       // declare a buffer to read blocks
    char placeholder[2 * MAXLEN];       // declare a placeholder to hold the strings stored within blocks
    // if the file is empty we allocate a block and we insert in it then we return 0 operation done successfully
    if(getTOVSHeader(*file,1) == 0){
        allocateTOVSBlock(file);                        // allocation
        readTOVSBlock(*file,blockNumber,&block);    // reading
        strcpy(block.info,parsedString);            // assign the record to the the field of info withing the buffer
        block.nb_records = 1;
        block.overlaps = 0;
        block.maxKey = data.key;
        block.minKey = data.key;
        setTOVSHeader(file,2,getTOVSHeader(*file,2) + strlen(parsedString));
        setTOVSHeader(file,3,0);
        setTOVSHeader(file,4,1);
        writeTOVSBlock(file,block,blockNumber); // re-write the block
        return 0;       // end of the algorithm
    }

    readTOVSBlock(*file,blockNumber,&block);    // we read the block whose index (number) was returned from the search
    int i = 0, j = 0;
    if(data.key > block.maxKey){  // if the record's key we're inserting is greater than all the other keys in the block we do what follows
        j = strlen(block.info);        // keep the length of the information saved already in the block
        int n = strlen(parsedString);
        if(j == MAXLEN - 1){   // if the block is completely filled
                                // we insert at the beginning of the next block
            blockNumber += 1;
            readTOVSBlock(*file,blockNumber,&block);
            char overlappedPart[100];          // we shall truncate the overlapped portion if it exists
            substring(block.info,'|',&i,overlappedPart);

            if(!isValid(overlappedPart)){       // if it's a valid string, then there is no overlapping, so we concatenate
                strcat(overlappedPart,"|");     // our strings right away then we call the writeAndShift function do write the new information
                strcpy(placeholder,overlappedPart);
                strcat(placeholder,parsedString);
                strcat(placeholder,sliceString(block.info,i,strlen(block.info)));
            } else {                                   // if it's not a valid string, we put our string, after the overlapping part, and before the first valid string in the
                strcpy(placeholder,parsedString);   // next block
                strcat(placeholder,block.info);
            }

            writeAndShift(file,placeholder,data,blockNumber,block);
        }
        else if(j + n < MAXLEN){       // if the sum of the length of the information we already have and the new information fits within the bounds
            block.maxKey = data.key;    // of the block, we concatenate and write directly
            block.nb_records += 1;
            strcat(block.info,parsedString);
            setTOVSHeader(file,2,getTOVSHeader(*file,2) + n);
            writeTOVSBlock(file,block,blockNumber);
        } else {           // if we still have some space, but it's not enough, we concatenate, and writeAndShift will do the rest
            strcpy(placeholder,block.info);
            strcat(placeholder,parsedString);
            writeAndShift(file,placeholder,data,blockNumber,block);
        }
    }
    else if(data.key < block.minKey){       // if we have to write at the beginning of a block, it's valid only when inserting in the first block
                                           // we concatenate ( newStr + oldStr ) then writeAndShift does the rest, just like in the previous case
        strcpy(placeholder,parsedString);
        strcat(placeholder,block.info);
        writeAndShift(file,placeholder,data,blockNumber,block);

    } else {
        strcpy(placeholder,sliceString(block.info,0,insertPosition));                    // if we have to insert in the middle of the block, it's even simpler
        strcat(placeholder,parsedString);                                                // the search procedure will return an index to insert at it, we use it to slice the old data, and put
        strcat(placeholder,sliceString(block.info,insertPosition , strlen(block.info))); // the new data in between (dat1|dat2) + (newDat) ==> (dat1|newDat|dat2) then we pass it to writeAndShift
        writeAndShift(file,placeholder,data,blockNumber,block);
    }
    setTOVSHeader(file,2,getTOVSHeader(*file,2) + strlen(parsedString)); // update the number of chars. we wrote
    setTOVSHeader(file,2,getTOVSHeader(*file,4) + 1);
    return 0;
}
