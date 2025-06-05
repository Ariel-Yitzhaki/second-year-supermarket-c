#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myMacros.h"
#include "General.h"
#include "FileHelper.h"
#include "SuperFile.h"
#include "Product.h"


int	saveSuperMarketToFile(const SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "wb");
	CHECK_MSG_RETURN_0(fp, "Failed opening file");
	printf("pointer check successful");

	if (!writeStringToFile(pMarket->name, fp, "Error write supermarket name\n"))
	{
		fclose(fp);
		return 0;
	}

	if (!writeIntToFile(pMarket->productCount, fp, "Error write product count\n"))
	{
		fclose(fp);
		return 0;
	}

	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (!saveProductToFile(pMarket->productArr[i], fp))
		{
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	saveCustomersToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName);

	return 1;
}

int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp, "Failed opening file");

	pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
	if (!pMarket->name)
	{
		fclose(fp);
		return 0;
	}

	int count;

	if (!readIntFromFile(&count, fp, "Error reading product count\n"))
	{
		free(pMarket->name);
		fclose(fp);
		return 0;
	}

	pMarket->productArr = (Product**)malloc(count * sizeof(Product*));
	if (!pMarket->productArr)
	{
		free(pMarket->name);
		fclose(fp);
		return 0;
	}

	pMarket->productCount = count;

	for (int i = 0; i < count; i++)
	{
		pMarket->productArr[i] = (Product*)malloc(sizeof(Product));
		if (!pMarket->productArr[i])
		{
			free(pMarket->name);
			fclose(fp);
			return 0;
		}

		if (!loadProductFromFile(pMarket->productArr[i], fp))
		{
			free(pMarket->productArr[i]);
			free(pMarket->name);
			fclose(fp);
			return 0;
		}
	}


	fclose(fp);

	pMarket->customerArr = loadCustomersFromTextFile(customersFileName, &pMarket->customerCount);
	if (!pMarket->customerArr)
		return 0;

	return	1;

}


int loadCompressedMarketFromFile(SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp, "Failed opening file");

	//read 2 bytes- product count and the market name
	char buffer[2] = { 0 };
	if (fread(buffer, sizeof(char), 2, fp) != 2)
	{
		CLOSE_RETURN_0(fp);
	}
	pMarket->productCount = buffer[0];
	pMarket->productCount <<= 2;
	pMarket->productCount |= (buffer[1] & 0xC0) >> 6;
	int name_len = buffer[1] & 0x3F;
	pMarket->name = (char*)malloc(sizeof(char) * (name_len + 1));
	if (!pMarket->name || fread(pMarket->name, sizeof(char), name_len, fp) != name_len)
	{
		printf("Error reading supermarket name\n");
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);

	}
	pMarket->name[name_len] = '\0';
	pMarket->productArr = (Product**)malloc(sizeof(Product*) * pMarket->productCount);
	if (!pMarket->productArr)
	{
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
	}
	for (int i = 0; i < pMarket->productCount; i++)
	{
		pMarket->productArr[i] = (Product*)malloc(sizeof(Product));
		if (!pMarket->productArr[i])
		{
			FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
		}
		char buffer2[4] = { 0 };
		if (fread(buffer2, sizeof(char), 4, fp) != 3)
		{
			CLOSE_RETURN_0(fp);
		}
		pMarket->productArr[i]->barcode[2] = ('0' + (buffer2[0] & 0xF0)) >> 4;
		pMarket->productArr[i]->barcode[3] = '0' + buffer2[0] & 0x0F;
		pMarket->productArr[i]->barcode[4] = ('0' + (buffer2[1] & 0xF0)) >> 4;
		pMarket->productArr[i]->barcode[5] = '0' + buffer2[1] & 0xF0;
		pMarket->productArr[i]->barcode[6] = ('0' + (buffer2[2] & 0xF0)) >> 4;
		pMarket->productArr[i]->barcode[7] = '\0';
		int type = (buffer2[2] & 0x0C) >> 2;
		pMarket->productArr[i]->type = (eProductType)type;
		strcpy(buffer, getProductTypePrefix((eProductType)type));
		pMarket->productArr[i]->barcode[0] = buffer[0];
		pMarket->productArr[i]->barcode[1] = buffer[1];
		name_len = buffer2[2] & 0x3;
		name_len <<= 6;
		name_len |= buffer2[3];
		if (fread(pMarket->productArr[i]->name, sizeof(char), name_len, fp) != name_len)
		{
			CLOSE_RETURN_0(fp);
		}
		pMarket->productArr[i]->name[name_len] = '\0';
		char buffer3[1] = { 0 };
		if (fread(buffer3, sizeof(char), 1, fp) != 1)
		{
			CLOSE_RETURN_0(fp);
		}
		pMarket->productArr[i]->count = buffer3[0] - '0';

		fread(buffer, sizeof(char), 2, fp);
		int change = (buffer[0] & 0xFE) >> 1;
		int no_change = buffer[0] & 0x01;
		no_change <<= 8;
		no_change |= buffer[1];
		pMarket->productArr[i]->price = no_change + (float)change / 100;

		fread(buffer, sizeof(char), 2, fp);
		pMarket->productArr[i]->expiryDate.day = (buffer[0] & 0xF8) >> 3;
		pMarket->productArr[i]->expiryDate.month = buffer[0] & 0x07;
		pMarket->productArr[i]->expiryDate.month <<= 1;
		pMarket->productArr[i]->expiryDate.month |= (buffer[1] & 0x80) >> 7;
		pMarket->productArr[i]->expiryDate.year = (2024 + (buffer[1] & 0x70)) >> 4;


	}

}

int	saveCustomersToTextFile(const Customer* customerArr, int customerCount, const char* customersFileName)
{
	FILE* fp;

	fp = fopen(customersFileName, "w");
	CHECK_RETURN_0(fp);

	fprintf(fp, "%d\n", customerCount);
	for (int i = 0; i < customerCount; i++)
		customerArr[i].vTable.saveToFile(&customerArr[i], fp);

	fclose(fp);
	return 1;
}

Customer* loadCustomersFromTextFile(const char* customersFileName, int* pCount)
{
	FILE* fp;

	fp = fopen(customersFileName, "r");
	CHECK_RETURN_0(fp);

	Customer* customerArr = NULL;
	int customerCount;

	fscanf(fp, "%d\n", &customerCount);

	if (customerCount > 0)
	{
		customerArr = (Customer*)calloc(customerCount, sizeof(Customer)); //cart will be NULL!!!
		if (!customerArr)
		{
			fclose(fp);
			return NULL;
		}

		for (int i = 0; i < customerCount; i++)
		{
			if (!loadCustomerFromFile(&customerArr[i], fp))
			{
				freeCustomerCloseFile(customerArr, i, fp);
				return NULL;
			}
		}
	}

	fclose(fp);
	*pCount = customerCount;
	return customerArr;
}


void freeCustomerCloseFile(Customer* customerArr, int count, FILE* fp)
{
	for (int i = 0; i < count; i++)
	{
		free(customerArr[i].name);
		customerArr[i].name = NULL;
		if (customerArr[i].pDerivedObj)
		{
			free(customerArr[i].pDerivedObj);
			customerArr[i].pDerivedObj = NULL;
		}
	}
	FREE_CLOSE_FILE_RETURN_0(customerArr, fp);
}