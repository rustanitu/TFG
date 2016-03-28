#ifndef LQC_SORT_QUICKSORT_H
#define	LQC_SORT_QUICKSORT_H

namespace lqc
{
  //https://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Ffunction_templates.htm
  template<class T>
  void QuickSort (T a[], const int& leftarg, const int& rightarg)
  {
    if (leftarg < rightarg)
    {
      T pivotvalue = a[leftarg];
      int left = leftarg - 1;
      int right = rightarg + 1;

      for (;;)
      {
        while (a[--right] > pivotvalue);
        while (a[++left] < pivotvalue);

        if (left >= right) break;

        T temp = a[right];
        a[right] = a[left];
        a[left] = temp;
      }

      int pivot = right;
      QuickSort (a, leftarg, pivot);
      QuickSort (a, pivot + 1, rightarg);
    }
  }
}

#endif