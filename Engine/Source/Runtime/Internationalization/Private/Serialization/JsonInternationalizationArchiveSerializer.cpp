﻿// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "InternationalizationPrivatePCH.h"
#include "Internationalization/InternationalizationArchive.h"
#include "Internationalization/InternationalizationMetadata.h"
#include "JsonInternationalizationArchiveSerializer.h"
#include "JsonInternationalizationMetadataSerializer.h"


DEFINE_LOG_CATEGORY_STATIC(LogInternationalizationArchiveSerializer, Log, All);


const FString FJsonInternationalizationArchiveSerializer::TAG_FORMATVERSION = TEXT("FormatVersion");
const FString FJsonInternationalizationArchiveSerializer::TAG_NAMESPACE = TEXT("Namespace");
const FString FJsonInternationalizationArchiveSerializer::TAG_CHILDREN = TEXT("Children");
const FString FJsonInternationalizationArchiveSerializer::TAG_SUBNAMESPACES = TEXT("Subnamespaces");
const FString FJsonInternationalizationArchiveSerializer::TAG_DEPRECATED_DEFAULTTEXT = TEXT("DefaultText");
const FString FJsonInternationalizationArchiveSerializer::TAG_DEPRECATED_TRANSLATEDTEXT = TEXT("TranslatedText");
const FString FJsonInternationalizationArchiveSerializer::TAG_OPTIONAL = TEXT("Optional");
const FString FJsonInternationalizationArchiveSerializer::TAG_SOURCE = TEXT("Source");
const FString FJsonInternationalizationArchiveSerializer::TAG_SOURCE_TEXT = TEXT("Text");
const FString FJsonInternationalizationArchiveSerializer::TAG_TRANSLATION = TEXT("Translation");
const FString FJsonInternationalizationArchiveSerializer::TAG_TRANSLATION_TEXT = FJsonInternationalizationArchiveSerializer::TAG_SOURCE_TEXT;
const FString FJsonInternationalizationArchiveSerializer::TAG_METADATA_KEY = TEXT("Key");
const FString FJsonInternationalizationArchiveSerializer::NAMESPACE_DELIMITER = TEXT(".");


struct FCompareArchiveEntryBySourceAndKey
{
	FORCEINLINE bool operator()( TSharedPtr< FArchiveEntry > A, TSharedPtr< FArchiveEntry > B ) const
	{
		bool bResult = false;
		if( A->Source < B->Source )
		{
			bResult = true;
		}
		else if( A->Source == B->Source )
		{
			if( A->KeyMetadataObj.IsValid() != B->KeyMetadataObj.IsValid() )
			{
				bResult = B->KeyMetadataObj.IsValid();
			}
			else if( A->KeyMetadataObj.IsValid() && B->KeyMetadataObj.IsValid() )
			{
				bResult = (*(A->KeyMetadataObj) < *(B->KeyMetadataObj));
			}
		}
		return bResult;
	}
};


struct FCompareStructuredArchiveEntryByNamespace
{
	FORCEINLINE bool operator()( TSharedPtr< FStructuredArchiveEntry > A, TSharedPtr< FStructuredArchiveEntry > B ) const
	{
		return A->Namespace < B->Namespace;
	}
};


#if 0 // @todo Json: Serializing from FArchive is currently broken
bool FJsonInternationalizationArchiveSerializer::DeserializeArchive( FArchive& Archive, TSharedRef< FInternationalizationArchive > InternationalizationArchive )
{
	TSharedPtr< FJsonObject > JsonArchiveObj;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create( &Archive );
	bool bExecSuccessful = FJsonSerializer::Deserialize( Reader, JsonArchiveObj );

	if (bExecSuccessful && JsonArchiveObj.IsValid())
	{
		bExecSuccessful = DeserializeInternal( JsonArchiveObj.ToSharedRef(), InternationalizationArchive );
	}		

	return bExecSuccessful;
}
#endif


bool FJsonInternationalizationArchiveSerializer::DeserializeArchive( const FString& InStr, TSharedRef< FInternationalizationArchive > InternationalizationArchive )
{
	TSharedPtr< FJsonObject > JsonArchiveObj;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create( InStr );
	bool bExecSuccessful = FJsonSerializer::Deserialize( Reader, JsonArchiveObj );

	if ( bExecSuccessful && JsonArchiveObj.IsValid() )
	{
		bExecSuccessful = DeserializeInternal( JsonArchiveObj.ToSharedRef(), InternationalizationArchive );
	}

	return bExecSuccessful;
}


bool FJsonInternationalizationArchiveSerializer::DeserializeArchive( TSharedRef< FJsonObject > InJsonObj, TSharedRef< FInternationalizationArchive > InternationalizationArchive )
{
	return DeserializeInternal( InJsonObj, InternationalizationArchive );
}


#if 0 // @todo Json: Serializing from FArchive is currently broken
bool FJsonInternationalizationArchiveSerializer::SerializeArchive( TSharedRef< const FInternationalizationArchive > InternationalizationArchive, FArchive& Archive )
{
	TSharedRef< FJsonObject > JsonArchiveObj = MakeShareable( new FJsonObject );
	bool bExecSuccessful = SerializeInternal( InternationalizationArchive, JsonArchiveObj );

	if( bExecSuccessful )
	{
		TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create( &Archive );
		bExecSuccessful = FJsonSerializer::Serialize( JsonArchiveObj, Writer );
		Writer->Close();
	}
	return bExecSuccessful;
}
#endif


bool FJsonInternationalizationArchiveSerializer::SerializeArchive( TSharedRef< const FInternationalizationArchive > InternationalizationArchive, FString& Str )
{
	TSharedRef< FJsonObject > JsonArchiveObj = MakeShareable( new FJsonObject );
	bool bExecSuccessful = SerializeInternal( InternationalizationArchive, JsonArchiveObj );

	if( bExecSuccessful )
	{
		TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create( &Str );
		bExecSuccessful = FJsonSerializer::Serialize( JsonArchiveObj, Writer );
		Writer->Close();
	}
	return bExecSuccessful;
}


bool FJsonInternationalizationArchiveSerializer::SerializeArchive( TSharedRef< const FInternationalizationArchive > InternationalizationArchive, TSharedRef< FJsonObject > JsonObj )
{
	return SerializeInternal( InternationalizationArchive, JsonObj );
}


bool FJsonInternationalizationArchiveSerializer::DeserializeInternal( TSharedRef< FJsonObject > InJsonObj, TSharedRef< FInternationalizationArchive > InternationalizationArchive )
{
	if( InJsonObj->HasField( TAG_FORMATVERSION ) )
	{
		const int32 FormatVersion = static_cast<int>(InJsonObj->GetNumberField( TAG_FORMATVERSION ));
		InternationalizationArchive->SetFormatVersion(static_cast<FInternationalizationArchive::EFormatVersion>(FormatVersion));
	}
	else
	{
		InternationalizationArchive->SetFormatVersion(FInternationalizationArchive::EFormatVersion::Initial);
	}

	return JsonObjToArchive( InJsonObj, TEXT(""), InternationalizationArchive );
}


bool FJsonInternationalizationArchiveSerializer::SerializeInternal( TSharedRef< const FInternationalizationArchive > InInternationalizationArchive, TSharedRef< FJsonObject > JsonObj )
{
	TSharedPtr< FStructuredArchiveEntry > RootElement = MakeShareable( new FStructuredArchiveEntry( TEXT("") ) );

	// Condition the data so that it exists in a structured hierarchy for easy population of the JSON object.
	GenerateStructuredData( InInternationalizationArchive, RootElement );

	SortStructuredData( RootElement );

	// Clear anything that may be in the JSON object
	JsonObj->Values.Empty();

	// Set format version.
	JsonObj->SetNumberField(TAG_FORMATVERSION, static_cast<double>(InInternationalizationArchive->GetFormatVersion()));

	// Setup the JSON object using the structured data created
	StructuredDataToJsonObj( RootElement, JsonObj );
	return true;
}


bool FJsonInternationalizationArchiveSerializer::JsonObjToArchive( TSharedRef< FJsonObject > InJsonObj, FString ParentNamespace, TSharedRef< FInternationalizationArchive > InternationalizationArchive )
{
	bool bConvertSuccess = true;
	FString AccumulatedNamespace = ParentNamespace;
	
	if( InJsonObj->HasField( TAG_NAMESPACE) )
	{
		if( !( AccumulatedNamespace.IsEmpty() ) )
		{
			AccumulatedNamespace += NAMESPACE_DELIMITER;
		}
		AccumulatedNamespace += InJsonObj->GetStringField( TAG_NAMESPACE );
	}
	else
	{
		UE_LOG( LogInternationalizationArchiveSerializer, Warning,TEXT("Encountered an object with a missing namespace while converting to Internationalization archive.") );
		bConvertSuccess = false;
	}

	// Process all the child objects
	if( bConvertSuccess && InJsonObj->HasField( TAG_CHILDREN ) )
	{
		const TArray< TSharedPtr<FJsonValue> > ChildrenArray = InJsonObj->GetArrayField( TAG_CHILDREN );

		for( TArray< TSharedPtr< FJsonValue > >::TConstIterator ChildIter( ChildrenArray.CreateConstIterator() ); ChildIter; ++ChildIter )
		{
			const TSharedPtr< FJsonValue >  ChildEntry = *ChildIter;
			const TSharedPtr< FJsonObject > ChildJSONObject = ChildEntry->AsObject();

			FString SourceText;
			TSharedPtr< FLocMetadataObject > SourceMetadata;
			if( ChildJSONObject->HasTypedField< EJson::String >( TAG_DEPRECATED_DEFAULTTEXT ) )
			{
				SourceText = ChildJSONObject->GetStringField( TAG_DEPRECATED_DEFAULTTEXT );
			} 
			else if( ChildJSONObject->HasTypedField< EJson::Object >( TAG_SOURCE ) )
			{
				const TSharedPtr< FJsonObject > SourceJSONObject = ChildJSONObject->GetObjectField( TAG_SOURCE );
				if( SourceJSONObject->HasTypedField< EJson::String >( TAG_SOURCE_TEXT ) )
				{
					SourceText = SourceJSONObject->GetStringField( TAG_SOURCE_TEXT );

					// Source meta data is mixed in with the source text, we'll process metadata if the source json object has more than one entry
					if( SourceJSONObject->Values.Num() > 1 )
					{
						// We load in the entire source object as metadata and just remove the source text.
						FJsonInternationalizationMetaDataSerializer::DeserializeMetadata( SourceJSONObject.ToSharedRef(), SourceMetadata );
						if( SourceMetadata.IsValid() )
						{
							SourceMetadata->Values.Remove( TAG_SOURCE_TEXT );
						}
					}
				}
				else
				{
					bConvertSuccess = false;
				}
			}
			else
			{
				bConvertSuccess = false;
			}

			FString TranslationText;
			TSharedPtr< FLocMetadataObject > TranslationMetadata;
			if( ChildJSONObject->HasTypedField< EJson::String >( TAG_DEPRECATED_TRANSLATEDTEXT ) )
			{
				TranslationText = ChildJSONObject->GetStringField( TAG_DEPRECATED_TRANSLATEDTEXT );
			} 
			else if( ChildJSONObject->HasTypedField< EJson::Object >( TAG_TRANSLATION ) )
			{
				const TSharedPtr< FJsonObject > TranslationJSONObject = ChildJSONObject->GetObjectField( TAG_TRANSLATION );
				if( TranslationJSONObject->HasTypedField< EJson::String >( TAG_TRANSLATION_TEXT ) )
				{
					TranslationText = TranslationJSONObject->GetStringField( TAG_TRANSLATION_TEXT );

					// Source meta data is mixed in with the source text, we'll process metadata if the source json object has more than one entry
					if( TranslationJSONObject->Values.Num() > 1 )
					{
						// We load in the entire source object as metadata and remove the source text
						FJsonInternationalizationMetaDataSerializer::DeserializeMetadata( TranslationJSONObject.ToSharedRef(), TranslationMetadata );
						if( TranslationJSONObject.IsValid() )
						{
							TranslationJSONObject->Values.Remove( TAG_TRANSLATION_TEXT );
						}
					}
				}
				else
				{
					bConvertSuccess = false;
				}
			}
			else
			{
				bConvertSuccess = false;
			}

			if( bConvertSuccess )
			{
				FLocItem Source( SourceText );
				Source.MetadataObj = SourceMetadata;

				FLocItem Translation( TranslationText );
				Translation.MetadataObj = TranslationMetadata;

				bool bIsOptional = false;
				if( ChildJSONObject->HasTypedField< EJson::Boolean >( TAG_OPTIONAL ) )
				{
					bIsOptional = ChildJSONObject->GetBoolField( TAG_OPTIONAL );
				}

				TSharedPtr< FLocMetadataObject > MetadataNode;
				if( ChildJSONObject->HasTypedField< EJson::Object >( TAG_METADATA_KEY ) )
				{
					const TSharedPtr< FJsonObject > MetaDataKeyJSONObject = ChildJSONObject->GetObjectField( TAG_METADATA_KEY );
					FJsonInternationalizationMetaDataSerializer::DeserializeMetadata( MetaDataKeyJSONObject.ToSharedRef(), MetadataNode );
				}

				bool bAddSuccessful = InternationalizationArchive->AddEntry( AccumulatedNamespace, Source, Translation, MetadataNode, bIsOptional );
				if( !bAddSuccessful )
				{
					UE_LOG( LogInternationalizationArchiveSerializer, Warning,TEXT("Could not add JSON entry to the Internationalization archive: Namespace:%s DefaultText:%s"), *AccumulatedNamespace, *SourceText );
				}
			}
		}
	}

	if( bConvertSuccess && InJsonObj->HasField( TAG_SUBNAMESPACES ) )
	{
		const TArray< TSharedPtr<FJsonValue> > SubnamespaceArray = InJsonObj->GetArrayField( TAG_SUBNAMESPACES );

		for(TArray< TSharedPtr< FJsonValue > >::TConstIterator SubnamespaceIter( SubnamespaceArray.CreateConstIterator() ); SubnamespaceIter; ++SubnamespaceIter )
		{
			const TSharedPtr< FJsonValue >  SubnamespaceEntry = *SubnamespaceIter;
			const TSharedPtr< FJsonObject > SubnamespaceJSONObject = SubnamespaceEntry->AsObject();

			if( !JsonObjToArchive( SubnamespaceJSONObject.ToSharedRef(), AccumulatedNamespace, InternationalizationArchive ) )
			{
				bConvertSuccess = false;
				break;
			}
		}
	}

	return bConvertSuccess;
}


void FJsonInternationalizationArchiveSerializer::GenerateStructuredData( TSharedRef< const FInternationalizationArchive > InInternationalizationArchive, TSharedPtr<FStructuredArchiveEntry> RootElement )
{
	//Loop through all the unstructured archive entries and build up our structured hierarchy
	for(TArchiveEntryContainer::TConstIterator It( InInternationalizationArchive->GetEntryIterator() ); It; ++It)
	{
		const TSharedRef< FArchiveEntry > UnstructuredArchiveEntry = It.Value();

		TArray< FString > NamespaceTokens;

		// Tokenize the namespace by using '.' as a delimiter
		int32 NamespaceTokenCount = UnstructuredArchiveEntry->Namespace.ParseIntoArray( &NamespaceTokens, *NAMESPACE_DELIMITER, true );

		TSharedPtr< FStructuredArchiveEntry > StructuredArchiveEntry = RootElement;
		//Loop through all the namespace tokens and find the appropriate structured entry, if it does not exist add it.  At the end StructuredArchiveEntry
		//  will point to the correct hierarchy entry for a given namespace
		for( int32 TokenIndex = 0; TokenIndex < NamespaceTokenCount; ++TokenIndex )
		{
			TSharedPtr<FStructuredArchiveEntry> FoundNamespaceEntry;
			for( int SubNamespaceIndex = 0; SubNamespaceIndex < StructuredArchiveEntry->SubNamespaces.Num(); SubNamespaceIndex++ )
			{
				if(  StructuredArchiveEntry->SubNamespaces[SubNamespaceIndex]->Namespace == NamespaceTokens[TokenIndex] )
				{
					FoundNamespaceEntry = StructuredArchiveEntry->SubNamespaces[SubNamespaceIndex];
					break;
				}
			}

			if( !FoundNamespaceEntry.IsValid() )
			{
				int32 index = StructuredArchiveEntry->SubNamespaces.Add( MakeShareable( new FStructuredArchiveEntry( NamespaceTokens[TokenIndex] ) ) );
				FoundNamespaceEntry = StructuredArchiveEntry->SubNamespaces[index];
			}
			StructuredArchiveEntry = FoundNamespaceEntry;
		}

		// We add the unstructured Archive entry to the hierarchy
		StructuredArchiveEntry->ArchiveEntries.AddUnique( UnstructuredArchiveEntry );
	}
}


void FJsonInternationalizationArchiveSerializer::SortStructuredData( TSharedPtr< FStructuredArchiveEntry > InElement )
{
	if( !InElement.IsValid() )
	{
		return;
	}

	// Sort the manifest entries by source text.
	InElement->ArchiveEntries.Sort( FCompareArchiveEntryBySourceAndKey() );

	// Sort the subnamespaces by namespace string
	InElement->SubNamespaces.Sort( FCompareStructuredArchiveEntryByNamespace() );

	// Do the sorting for each of the subnamespaces
	for( TArray< TSharedPtr< FStructuredArchiveEntry > >::TIterator Iter( InElement->SubNamespaces.CreateIterator() ); Iter; ++Iter )
	{
		TSharedPtr< FStructuredArchiveEntry > SubElement = *Iter;

		SortStructuredData( SubElement );
	}
}


void FJsonInternationalizationArchiveSerializer::StructuredDataToJsonObj( TSharedPtr< const FStructuredArchiveEntry > InElement, TSharedRef< FJsonObject > OutJsonObj )
{
	OutJsonObj->SetStringField( TAG_NAMESPACE, InElement->Namespace );

	TArray< TSharedPtr< FJsonValue > > NamespaceArray;
	TArray< TSharedPtr< FJsonValue > > EntryArray;

	//Write namespace content entries
	for( TArray< TSharedPtr< FArchiveEntry > >::TConstIterator Iter( InElement->ArchiveEntries ); Iter; ++Iter )
	{
		const TSharedPtr< FArchiveEntry > Entry = *Iter;
		TSharedPtr< FJsonObject > EntryNode = MakeShareable( new FJsonObject );

		FString ProcessedSourceText = Entry->Source.Text;
		FString ProcessedTranslation = Entry->Translation.Text;

		TSharedPtr< FJsonObject > SourceNode;
		if( Entry->Source.MetadataObj.IsValid() )
		{
			FJsonInternationalizationMetaDataSerializer::SerializeMetadata( Entry->Source.MetadataObj.ToSharedRef(), SourceNode );
		}

		if( !SourceNode.IsValid() )
		{
			SourceNode = MakeShareable( new FJsonObject );
		}

		SourceNode->SetStringField( TAG_SOURCE_TEXT, ProcessedSourceText );
		EntryNode->SetObjectField( TAG_SOURCE, SourceNode );
		
		TSharedPtr< FJsonObject > TranslationNode;
		if( Entry->Translation.MetadataObj.IsValid() )
		{
			FJsonInternationalizationMetaDataSerializer::SerializeMetadata( Entry->Translation.MetadataObj.ToSharedRef(), TranslationNode );
		}

		if( !TranslationNode.IsValid() )
		{
			TranslationNode = MakeShareable( new FJsonObject );
		}

		TranslationNode->SetStringField( TAG_TRANSLATION_TEXT, ProcessedTranslation );
		EntryNode->SetObjectField( TAG_TRANSLATION, TranslationNode );

		if( Entry->KeyMetadataObj.IsValid() )
		{
			TSharedPtr< FJsonObject > KeyDataNode;
			FJsonInternationalizationMetaDataSerializer::SerializeMetadata( Entry->KeyMetadataObj.ToSharedRef(), KeyDataNode );
			if( KeyDataNode.IsValid() )
			{
				EntryNode->SetObjectField( TAG_METADATA_KEY, KeyDataNode );
			}
		}

		// We only add the optional field if it is true, it is assumed to be false otherwise.
		if( Entry->bIsOptional == true )
		{
			EntryNode->SetBoolField( TAG_OPTIONAL, Entry->bIsOptional );
		}

		EntryArray.Add( MakeShareable( new FJsonValueObject( EntryNode ) ) );
	}

	//Write the subnamespaces
	for( TArray< TSharedPtr< FStructuredArchiveEntry > >::TConstIterator Iter( InElement->SubNamespaces ); Iter; ++Iter )
	{
		const TSharedPtr<FStructuredArchiveEntry> SubElement = *Iter;
		if( SubElement.IsValid() )
		{
			TSharedRef<FJsonObject> SubObject = MakeShareable( new FJsonObject );
			StructuredDataToJsonObj( SubElement, SubObject );

			NamespaceArray.Add( MakeShareable( new FJsonValueObject( SubObject ) ) );
		}
	}

	if( EntryArray.Num() > 0 )
	{
		OutJsonObj->SetArrayField( TAG_CHILDREN, EntryArray );
	}

	if( NamespaceArray.Num() > 0 )
	{
		OutJsonObj->SetArrayField( TAG_SUBNAMESPACES, NamespaceArray );
	}
}
