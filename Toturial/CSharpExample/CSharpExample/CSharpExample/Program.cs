using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpExample
{
    class Program
    {
        static void Main(string[] args)
        {
            SampleGeneric samGen = new SampleGeneric();
            samGen.TestList();
            samGen.TestQueue();
            samGen.TestStack();

            while (true)
            {

            }
        }
    }
}
