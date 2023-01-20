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
      echo "You entered $column under the s flag"
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
if [ ! -s main.csv ]; then
    echo "DATE,CATEGORY,AMOUNT,NAME" >> main.csv
fi

if [ -n "$category" ]
then
    awk -F"," -v awkcategory="$category" 'FNR>1 && $2==awkcategory{amt=amt+$3} END{printf("Total amount of category %s is %Lf\n",awkcategory,amt)}' main.csv

elif [ -n "$name" ]
then
    awk -F"," -v awkname="$name" 'FNR>1 && $4==awkname{amt=amt+$3} END{printf("Total expenditure of %s is %Lf\n",awkname,amt)}' main.csv

elif [ -n "$column" ]
then
    case "$column" in
    "date")
      (head -n1 main.csv && tail -n+2 main.csv | sort -t, -k1) > temp.csv
    ;;
    "category")
      (head -n1 main.csv && tail -n+2 main.csv | sort -t, -k2) > temp.csv
    ;;
    "amount")
      (head -n1 main.csv && tail -n+2 main.csv | sort -t, -k3) > temp.csv
    ;;
    "name")
      (head -n1 main.csv && tail -n+2 main.csv | sort -t, -k4) > temp.csv
    ;;
    *)
      echo "Unknown column type"
      exit 0
    ;;
    esac
    cat temp.csv>main.csv
    rm temp.csv
else
     echo "$1,$2,$3,$4" >>main.csv
     (head -n1 main.csv && tail -n+2 main.csv | sort -t, -k1) > temp.csv
     cat temp.csv>main.csv
     rm temp.csv
fi

