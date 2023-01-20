#!/bin/bash
while getopts ":c:n:s:h" ops;
do
  case $ops in
    c)
      category="$OPTARG"
      ;;
    n)
      name="$OPTARG"
      ;;
    s)
      column="$OPTARG"
      ;;
    h)
      echo "UTILITY NAME: CSVFileManip"
      echo ""
      echo "DESCRIPTION: Data handler script for the main.csv file that stores (date,category,amount,expenses)"
      echo "Can calculate total expense of a person, total amount spent under a category, and sort the csv based on the chosen field"
      echo ""
      echo "SYNOPSIS: bash <script_name>.sh [-h] [-c category] [-n name] [-s column] new_date new_category new_amount new_expense"
      exit 0
      ;;
    \?)
      echo "Unknown option: -$OPTARG"
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument."
      exit 1
      ;;
  esac
done

shift $((OPTIND-1))
touch "main.csv"
echo "$1,$2,$3,$4" >> main.csv

if [ -n "$category" ]
then
    awk -F"," -v awkcategory="$category" 'FNR>1 && $2==awkcategory{amt=amt+$3} END{printf("Total amount of category %s is %Lf\n",awkcategory,amt)}' main.csv
fi
if [ -n "$name" ]
then
    awk -F"," -v awkname="$name" 'FNR>1 && $4==awkname{amt=amt+$3} END{printf("Total expenditure of %s is %Lf\n",awkname,amt)}' main.csv
fi
if [ -n "$column" ]
then
    case "$column" in
    "date")
      sort -t, -k1 main.csv > temp.csv
    ;;
    "category")
      sort -t, -k2 main.csv > temp.csv
    ;;
    "amount")
      sort -t, -k3 main.csv > temp.csv
    ;;
    "name")
      sort -t, -k4 main.csv > temp.csv
    ;;
    *)
      echo "Unknown column type"
      exit 1
    ;;
    esac
else
    sort -t, -k1 main.csv > temp.csv
fi
cat temp.csv > main.csv
rm temp.csv