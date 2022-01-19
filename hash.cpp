
#include "hash.h"
#include "math.h"
const float LOAD_FACTOR_REHASH = .5;
const float DELETED_RATIO_REHASH = .8;
const int ZERO = 0;
const int FOUR = 4;
const float TWENTY_FIVE_PERCENT = .25;

HashTable::HashTable(unsigned size, hash_fn hash){
    if(size < MINPRIME){
        size = MINPRIME;
    }if(size > MAXPRIME){
        size = MAXPRIME;
    }if(!isPrime(size)){
        size = findNextPrime(size);
    }
        m_currentlyHashing = false;
        m_numDeleted1 = ZERO;
        m_numDeleted2 = ZERO;
        m_size1 = ZERO;
        m_size2 = ZERO;
        m_capacity1 = size;
        m_capacity2 = ZERO;
        m_table1 = new File [m_capacity1];
        m_table2 = nullptr;
        m_hash = hash;
        m_newTable = TABLE1;
}

HashTable::~HashTable(){
    m_hash = nullptr;
    m_size1 = ZERO;
    m_size2 = ZERO;
    m_capacity1 = ZERO;
    m_capacity2 = ZERO;
    m_numDeleted1 = ZERO;
    m_numDeleted2 = ZERO;
    m_currentlyHashing = false;

    if(m_table2 != nullptr){
        delete [] m_table2;
    }if(m_table1 != nullptr){
        delete [] m_table1;
    }
}

File HashTable::getFile(string name, unsigned int diskBlock){
    // null file to return if getFile doesnt find file
    File nullFile;
    // searching both tables if in middle of rehash
    if(m_table1 != nullptr){
        for (unsigned int i = 0; i < m_capacity1; i++){
            // if the index isnt empty then we search it for the value
            if(!m_table1[i].key().empty()){
                if(m_table1[i].key() == name && m_table1[i].diskBlock() == diskBlock){
                    return m_table1[i];
                }
            }
        }
    }if(m_table2 != nullptr){
        for (unsigned int i = 0; i < m_capacity2; i++){
            if(!m_table2[i].key().empty()){
                if(m_table2[i].key() == name && m_table2[i].diskBlock() == diskBlock){
                    return m_table2[i];
                }
            }
        }
    }
    cout << "File not found" << endl;
    return nullFile;
}

bool HashTable::insert(File file){
    bool insertSuccess = false;
    // working with table1
    if(m_newTable == TABLE1){
        int m_quadraticCounter = ZERO;
        unsigned int hashNum = m_hash(file.key());
        hashNum = hashNum % m_capacity1;
        // temp to keep hold of orginal hash value to refrenence back once hashNum is changed
         const unsigned int tempNum = hashNum;
        if(m_table1[hashNum].key().empty()){
            m_table1[hashNum] = file;
            m_size1++;
            insertSuccess = true;
        }else{
            // keep searching for an open index while doing quadratic probing this can be empty or deleted key
            while(!m_table1[hashNum].key().empty() && !(m_table1[hashNum] == DELETED)){
                hashNum = tempNum;
                hashNum = (hashNum + (m_quadraticCounter * m_quadraticCounter)) % m_capacity1;
                m_quadraticCounter++;
            }
            m_table1[hashNum] = file;
            m_size1++;
            insertSuccess = true;
        }
            if(m_currentlyHashing == true){
                reHash(TABLE2);
            }else{
                    if(lambda(TABLE1) > LOAD_FACTOR_REHASH){
                        createNewHash(TABLE1);
                        m_currentlyHashing = true;
                }
            }
    // working with table 2 everything else is exact same has code above
    }else{
        int m_quadraticCounter = ZERO;
        unsigned int hashNum = m_hash(file.key());
        hashNum = hashNum % m_capacity2;
        const unsigned int tempNum = hashNum;
        if(m_table2[hashNum].key().empty()){
        hashNum = (hashNum + (m_quadraticCounter * m_quadraticCounter)) % m_capacity2;
            m_table2[hashNum] = file;
            m_size2++;
            insertSuccess = true;
        }else{
            while(!m_table2[hashNum].key().empty() && !(m_table2[hashNum] == DELETED)){
                hashNum = tempNum;
                hashNum = (hashNum + (m_quadraticCounter * m_quadraticCounter)) % m_capacity2;
                m_quadraticCounter++;
            }
            m_table2[hashNum] = file;
            m_size2++;
            insertSuccess = true;
        }
             if(m_currentlyHashing == true){
                reHash(TABLE1);
            }else{
                    if(lambda(TABLE2) > LOAD_FACTOR_REHASH){
                        createNewHash(TABLE2);
                        m_currentlyHashing = true;
                }
            }
            
    }
    return insertSuccess;
}

bool HashTable::remove(File file){
    // if both tables exist during a remove must check both
    bool removeSuccess = false;
    if(m_table1 != nullptr){
        for (unsigned int i = 0; i < m_capacity1; i++){
        // if index isnt empty then we search for the file
        if(!m_table1[i].key().empty()){
            if(m_table1[i] == file){
                    m_table1[i] = DELETED;
                    m_numDeleted1++;
                    removeSuccess = true;
                }
            }
        // if we are currently working with table1 rehash table2
        }if(m_newTable == TABLE1){
            if(m_currentlyHashing == true){
                reHash(TABLE2);
            }else if(!m_currentlyHashing){
                if(deletedRatio(TABLE1) > DELETED_RATIO_REHASH){
                    m_currentlyHashing = true;
                    createNewHash(TABLE1);
                }
            }
        }
    }if(m_table2 != nullptr){
        for (unsigned int i = 0; i < m_capacity2; i++){
            if(!m_table2[i].key().empty()){
                if(m_table2[i] == file){
                    m_table2[i] = DELETED;
                    m_numDeleted2++;
                    removeSuccess = true;
                }
            }
        }
        // if working with table2 rehash table1
        if(m_newTable == TABLE2){
             if(m_currentlyHashing == true){
                reHash(TABLE1);
            }else if(!m_currentlyHashing){
                    if(deletedRatio(TABLE2) > DELETED_RATIO_REHASH){
                        m_currentlyHashing = true;
                        createNewHash(TABLE2);
                        
                }
            }
        } 
    }
    // if file was not found will return false
    return removeSuccess;
}

float HashTable::lambda(TABLENAME tablename) const {
    if(tablename == TABLE1){
        float size = float(m_size1);
        float capacity = float(m_capacity1);
        // finds lambda and returns it in float form
        return size / capacity;
    }else{
        float size = float(m_size2);
        float capacity = float(m_capacity2);
        return size / capacity;
    }
       
}

float HashTable::deletedRatio(TABLENAME tableName) const {
    float numDelete = 0.0;
    if(tableName == TABLE1){
        // count number of deleted keys
        for (unsigned int i = 0; i < m_capacity1; i++){
            if(!m_table1[i].key().empty()){
                if(m_table1[i].key() == DELETEDKEY){
                    numDelete += 1.0;
                }
            }
        }
        // take deleted key / occupied buckets
        float capacity = float(m_size1);
        return numDelete / capacity;
    }else{
        for (unsigned int i = 0; i < m_capacity2; i++){
            if(!m_table2[i].key().empty()){
                if(m_table2[i].key() == DELETEDKEY){
                    numDelete++;
                }
            }
        }
       float capacity = float(m_size2);
        return numDelete / capacity;
    }
}

void HashTable::dump() const {
    cout << "Dump for table 1: " << endl;
    if (m_table1 != nullptr)
        for (int i = 0; i < m_capacity1; i++) {
            cout << "[" << i << "] : " << m_table1[i] << endl;
        }
    cout << "Dump for table 2: " << endl;
    if (m_table2 != nullptr)
        for (int i = 0; i < m_capacity2; i++) {
            cout << "[" << i << "] : " << m_table2[i] << endl;
        }
}

bool HashTable::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int HashTable::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

void HashTable::reHash(TABLENAME tablename){
    if(tablename == TABLE1){
        // takes floor of 25% of currrent size
        int numEntrees = m_size1 * TWENTY_FIVE_PERCENT;
        unsigned int i = ZERO;
        int currentEntrees = ZERO;
        // will keep going until 25% is hit or we iterate through entire table
        while (i < m_capacity1 && currentEntrees <= numEntrees){
            if(!m_table1[i].key().empty()){
                if(m_table1[i].key() != DELETEDKEY){
                    // create temp file to deep copy objects on new table
                    File temp(m_table1[i].key(),m_table1[i].diskBlock());
                    insertHelper(temp);
                    m_table1[i] = DELETED;
                    currentEntrees++;
                }
            }
            i++;
        }
        // indicates rehash is compelete because we iterated entire table and moved everything over already
        if(i == m_capacity1){
            m_currentlyHashing = false;
            delete [] m_table1;
            m_size1 = ZERO;
            m_numDeleted1 = ZERO;
            m_table1 = nullptr;
            m_capacity1 = ZERO;
        }
    }
    else{
        int numEntrees = m_size2 * TWENTY_FIVE_PERCENT;
        unsigned int i = ZERO;
        int currentEntrees = ZERO;
        while (i < m_capacity2 && currentEntrees <= numEntrees){
            if(!m_table2[i].key().empty()){
                if(m_table2[i].key() != DELETEDKEY){
                    File temp(m_table2[i].key(),m_table2[i].diskBlock());
                    insertHelper(temp);
                    m_table2[i] = DELETED;
                    currentEntrees++;
                }
            }
            i++;
        }
        // indicates rehash is compelete
        if(i == m_capacity2){
            m_numDeleted2 = ZERO;
            m_currentlyHashing = false;
            m_size2 = ZERO;
            delete [] m_table2;
            m_table2 = nullptr;
            m_capacity2 = ZERO;
        }
    }
}

void HashTable::createNewHash(TABLENAME tablename){
    if(tablename == TABLE1){
        // if table2 somehow isnt cleared clear it first
        if(m_table2 != nullptr){
            delete [] m_table2;
            m_table2 = nullptr;
        }
            m_capacity2 = ((m_size1 - m_numDeleted1) * FOUR);
        // find a prime number for new capacity
        if(!isPrime(m_capacity2)){
            m_capacity2 = findNextPrime(m_capacity2);
        }
        // allocate the memory needed and set the hash into action
        cout << "Rehash has started" << endl;
        m_table2 = new File[m_capacity2];
        m_newTable = TABLE2;
        m_currentlyHashing = true;
    }else{
        // works same has code above just allocates for table1
        if(m_table1 != nullptr){
            delete [] m_table1;
        }
        m_capacity1 = ((m_size2 - m_numDeleted2) * FOUR);
        if(!isPrime(m_capacity1)){
            m_capacity1 = findNextPrime(m_capacity1);
        }
        cout << "Rehash has started" << endl;
        m_table1 = new File[m_capacity1];
        m_newTable = TABLE1;
        m_currentlyHashing = true;
    }
}

// works exactly like other insert function but doesnt check and do rehashing in here
void HashTable::insertHelper(File file){
       if(m_newTable == TABLE1){
        int m_quadraticCounter = ZERO;
        unsigned int hashNum = m_hash(file.key());
        hashNum = hashNum % m_capacity1;
        if(m_table1[hashNum].key().empty()){
            m_table1[hashNum] = file;
            m_size1++;
        }else{
            while(!m_table1[hashNum].key().empty() && !(m_table1[hashNum] == DELETED)){
                hashNum = (hashNum + (m_quadraticCounter * m_quadraticCounter)) % m_capacity1;
                m_quadraticCounter++;
            }
            m_table1[hashNum] = file;
            m_size1++;
        }
    // working with table 2
    }else{
        int m_quadraticCounter = ZERO;
        unsigned int hashNum = m_hash(file.key());
        hashNum = hashNum % m_capacity2;
        if(m_table2[hashNum].key().empty()){
        hashNum = (hashNum + (m_quadraticCounter * m_quadraticCounter)) % m_capacity2;
            m_table2[hashNum] = file;
            m_size2++;
        }else{
            while(!m_table2[hashNum].key().empty() && !(m_table2[hashNum] == DELETED)){
                hashNum = (hashNum + (m_quadraticCounter * m_quadraticCounter)) % m_capacity2;
                m_quadraticCounter++;
            }
            m_table2[hashNum] = file;
            m_size2++;
        }
    }
}