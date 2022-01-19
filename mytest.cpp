#include "hash.h"
#include <iostream>
#include <random>
#include <vector>
using namespace std;
enum RANDOM {UNIFORM, NORMAL};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORM) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 and standard deviation of 20 
            m_normdist = std::normal_distribution<>(50,20);
        }
        else{
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
    }

    int getRandNum(){
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else{
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//uniform distribution

};

// The hash function used by HashTable class
unsigned int hashCode(const string str);

class Tester{ // Tester class to implement test functions
   public:
   Tester(){}
   ~Tester(){}
   void testNonColliding();
   void testColliding();
   private:
};

int main(){
   // This program presents a sample use of the class HashTable 
   // It does not represent any rehashing
   Tester test;
   test.testNonColliding();
   test.testColliding();

   return 0;
}

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for ( int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}

// does all the test for a non colliding table
void Tester::testNonColliding(){
    Random diskBlockGen(DISKMIN,DISKMAX);
    int tempDiskBlocks[30] = {0};
    HashTable newTable(MINPRIME,hashCode);
    int temp = 0;

    for (int i = 0; i < 30; i++){
        temp = diskBlockGen.getRandNum();
        tempDiskBlocks[i] = temp;
    }

    cout << endl;
    cout << "Testing insertion with no colliding keys" << endl;
    File removeFile("driver.cpp",tempDiskBlocks[0]);
    File secondRemove("heap.h",tempDiskBlocks[4]);
    File thridRemove("mydriver.cpp",tempDiskBlocks[5]);
    newTable.insert(removeFile);
    newTable.insert(File("test.txt",tempDiskBlocks[1]));
    newTable.insert(File("mytest.cpp",tempDiskBlocks[2]));
    newTable.insert(File("heap.cpp",tempDiskBlocks[3]));
    newTable.insert(secondRemove);
    newTable.insert(thridRemove);
    newTable.insert(File("anothertest.txt",tempDiskBlocks[6]));
    newTable.insert(File("testfile",tempDiskBlocks[7]));

    unsigned int hashNum = hashCode("driver.cpp");
    hashNum = hashNum % MINPRIME;

    cout << "File driver.cpp should be inserted at index " << hashNum << endl;
    cout << endl;

    hashNum = hashCode("mytest.cpp");
    hashNum = hashNum % MINPRIME;

    cout << "File mytest.cpp should be inserted at index " << hashNum << endl;
    cout << endl;

    cout << "Dumping insertion of 8 non colliding files" << endl; 
    newTable.dump();
    cout << endl;

    cout << "Verifying m_size is correctly updated" << endl;
    cout << "Number of entrees in table => "<< newTable.numEntries(TABLE1) << endl;
    cout << endl;

    cout << "Testing find operation with non colliding files" << endl;
    File tempFile;

    tempFile = newTable.getFile("driver.cpp", tempDiskBlocks[0]);
    if(!(tempFile.key() == "" || tempFile.diskBlock() == 0)){
        cout << "file1 was found!" << endl;
        cout << tempFile.key() << endl;
        cout << endl;
    }
     tempFile = newTable.getFile("mydriver.cpp", tempDiskBlocks[5]);
    if(!(tempFile.key() == "" || tempFile.diskBlock() == 0)){
        cout << "file6 was found!" << endl;
        cout << tempFile.key() << endl;
        cout << endl;
    }
     tempFile = newTable.getFile("mytest.cpp", tempDiskBlocks[2]);
    if(!(tempFile.key() == "" || tempFile.diskBlock() == 0)){
        cout << "file3 was found!" << endl;
        cout << tempFile.key() << endl;
        cout << endl;
    }

    cout << "Testing removal of a few non colliding files" << endl; 
    cout << "Removing " << removeFile.key() << " => " << removeFile.diskBlock() << endl;
    if(newTable.remove(removeFile)){
        cout << "removal successful" << endl;
    }
    cout << endl;
    cout << "Removing " << secondRemove.key() << " => " << secondRemove.diskBlock() << endl;
    if(newTable.remove(secondRemove)){
        cout << "removal successful" << endl;
    }
    cout << endl;
    cout << "Removing " << thridRemove.key() << " => " << thridRemove.diskBlock() << endl;
    if(newTable.remove(thridRemove)){
        cout << "removal successful" << endl;
    }
    cout << endl;

    cout << "After removal of 3 non colliding files" << endl;
    newTable.dump();
    cout << endl;

    File extraTest("justatest.txt", tempDiskBlocks[25]);
    cout << "Testing removal of file not in map" << endl;
    newTable.remove(extraTest);
    cout << endl;
}

// does all the testing for a colliding table
void Tester::testColliding(){
{
    cout << "Testing insertion of colliding keys without triggering a rehash" << endl;
    cout << endl;
    Random diskBlockGen(DISKMIN,DISKMAX);
    int tempDiskBlocks[50] = {0};
    HashTable aTable(MINPRIME,hashCode);
    int temp = 0;
    int secondIndex = 0;
   for (int i=0;i<50;i++){
      temp = diskBlockGen.getRandNum();
      if (i%3 == 0){//this saves 17 numbers from the index range [0-49]
         tempDiskBlocks[secondIndex] = temp;
         secondIndex++;
      }
      if (i%3 != 0)
         aTable.insert(File("test.txt", temp));
      else
         // these will be deleted
         aTable.insert(File("driver.cpp", temp));
   }

   cout << "Message: dump after 50 insertions in a table with MINPRIME (101) buckets:" << endl;
   aTable.dump();

    cout << endl;
    cout << "Testing removal without triggering rehash" << endl;
    cout << endl;
   for (int i = 0;i<14;i++)
         aTable.remove(File("driver.cpp", tempDiskBlocks[i]));
   cout << "Message: dump after removing 14 buckets," << endl;
   aTable.dump();
}


    cout << endl;
    cout << "Testing insertion of colliding keys with triggering a rehash" << endl;
    cout << endl;

    HashTable rehashTable(MINPRIME, hashCode);
    Random diskBlockGen(DISKMIN,DISKMAX);
    int temp = 0;
    int secondIndex = 0;
    int tempDiskBlocks[100] = {0};

    cout << "Inserting 100 in a table with MINPRIME (101) buckets:" << endl;
    for (int i=0;i<100;i++){
      temp = diskBlockGen.getRandNum();
        tempDiskBlocks[secondIndex] = temp;
        secondIndex++;
        rehashTable.insert(File("driver.cpp", temp));
   }

   cout << "Dump of table after insertion and rehashing occured" << endl;
   rehashTable.dump();

   cout << "Testing removal of colliding keys with triggering a rehash" << endl;

   for (int i = 0; i < 84; i++){
       rehashTable.remove(File("driver.cpp",tempDiskBlocks[i]));
   }

   cout << endl;
   cout << "Dump of table after removal of more than 80 percent nodes and rehashing occured" << endl;

   rehashTable.dump();
}