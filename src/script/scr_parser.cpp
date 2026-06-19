#include "../qcommon/qcommon.h"
#include "script_public.h"

scrParserGlob_t scrParserGlob;
scrParserPub_t scrParserPub;

/*
==============
Scr_GetSourceBuffer
==============
*/
unsigned int Scr_GetSourceBuffer( const char *codePos )
{
	unsigned int bufferIndex;

	assert( Scr_IsInScriptMemory( codePos ) );
	assert( scrParserPub.sourceBufferLookupLen > 0 );

	for ( bufferIndex = scrParserPub.sourceBufferLookupLen - 1; bufferIndex; bufferIndex-- )
	{
		if ( scrParserPub.sourceBufferLookup[bufferIndex].codePos )
		{
			if ( scrParserPub.sourceBufferLookup[bufferIndex].codePos <= codePos )
			{
				break;
			}
		}
	}

	return bufferIndex;
}

/*
==============
Scr_HasSourceFiles
==============
*/
int Scr_HasSourceFiles( void )
{
	UNIMPLEMENTED(__FUNCTION__);
	return 0;
}

/*
==============
Scr_GetClosestSourcePosOfType
==============
*/
unsigned int Scr_GetClosestSourcePosOfType( unsigned int bufferIndex, unsigned int sourcePos, int type )
{
	const char *codePos, *opcodePos;
	unsigned int bestSourcePos;
	SourceBufferInfo *sourceBufData, *nextSourceBufData;
	SourceLookup *sourcePosLookup;
	int i, j;

	bestSourcePos = 0;
	sourceBufData = &scrParserPub.sourceBufferLookup[bufferIndex];

	if ( bufferIndex + 1 < scrParserPub.sourceBufferLookupLen )
		nextSourceBufData = &scrParserPub.sourceBufferLookup[bufferIndex + 1];
	else
		nextSourceBufData = NULL;

	if ( !sourceBufData->codePos )
	{
		return 0;
	}

	if ( nextSourceBufData )
		codePos = nextSourceBufData->codePos - 1;
	else
		codePos = NULL;

	for ( i = 0; i < scrParserGlob.opcodeLookupLen; i++ )
	{
		for ( j = 0; j < scrParserGlob.opcodeLookup[i].sourcePosCount; j++ )
		{
			opcodePos = scrParserGlob.opcodeLookup[i].codePos;

			if ( opcodePos < sourceBufData->codePos )
			{
				continue;
			}

			if ( opcodePos > codePos )
			{
				continue;
			}

			sourcePosLookup = &scrParserGlob.sourcePosLookup[scrParserGlob.opcodeLookup[i].sourcePosIndex + j];

			if ( ( sourcePosLookup->type & type ) != type )
			{
				continue;
			}

			if ( sourcePosLookup->sourcePos < bestSourcePos )
			{
				continue;
			}

			if ( sourcePosLookup->sourcePos > sourcePos )
			{
				continue;
			}

			bestSourcePos = sourcePosLookup->sourcePos;
		}
	}

	return bestSourcePos;
}

/*
==============
Scr_GetOpcodePosOfType
==============
*/
const char* Scr_GetOpcodePosOfType( unsigned int bufferIndex, unsigned int startSourcePos, unsigned int endSourcePos, int type, unsigned int *sourcePos )
{
	const char *codePos, *opcodePos;
	SourceBufferInfo *sourceBufData, *nextSourceBufData;
	SourceLookup *sourcePosLookup;
	unsigned int firstSourcePos;
	const char *firstOpcodePos;
	int i, j;

	sourceBufData = &scrParserPub.sourceBufferLookup[bufferIndex];

	if ( bufferIndex + 1 < scrParserPub.sourceBufferLookupLen )
		nextSourceBufData = &scrParserPub.sourceBufferLookup[bufferIndex + 1];
	else
		nextSourceBufData = NULL;

	if ( !sourceBufData->codePos )
	{
		*sourcePos = 0;
		return 0;
	}

	firstOpcodePos = 0;
	firstSourcePos = 0;

	if ( nextSourceBufData )
		codePos = nextSourceBufData->codePos - 1;
	else
		codePos = NULL;

	for ( i = 0; i < scrParserGlob.opcodeLookupLen; i++ )
	{
		for ( j = 0; j < scrParserGlob.opcodeLookup[i].sourcePosCount; j++ )
		{
			opcodePos = scrParserGlob.opcodeLookup[i].codePos;

			if ( opcodePos < sourceBufData->codePos )
			{
				continue;
			}

			if ( opcodePos > codePos )
			{
				continue;
			}

			sourcePosLookup = &scrParserGlob.sourcePosLookup[scrParserGlob.opcodeLookup[i].sourcePosIndex + j];

			if ( ( sourcePosLookup->type & type ) != type )
			{
				continue;
			}

			if ( sourcePosLookup->sourcePos < startSourcePos )
			{
				continue;
			}

			if ( sourcePosLookup->sourcePos >= endSourcePos )
			{
				continue;
			}

			if ( firstOpcodePos )
			{
				if ( scrParserGlob.opcodeLookup[i].codePos >= firstOpcodePos )
				{
					continue;
				}
			}

			firstOpcodePos = scrParserGlob.opcodeLookup[i].codePos;
			firstSourcePos = sourcePosLookup->sourcePos;
		}
	}

	*sourcePos = firstSourcePos;
	return firstOpcodePos;
}

/*
==============
AddThreadStartOpcodePos
==============
*/
void AddThreadStartOpcodePos( unsigned int sourcePos )
{
	SourceLookup *sourcePosLookup;

	if ( !scrVarPub.developer )
	{
		return;
	}

	if ( scrCompilePub.developer_statement == SCR_DEV_IGNORE )
	{
		assert(!scrVarPub.developer_script);
		return;
	}

	assert(scrParserGlob.threadStartSourceIndex >= 0);

	sourcePosLookup = &scrParserGlob.sourcePosLookup[scrParserGlob.threadStartSourceIndex];
	sourcePosLookup->sourcePos = sourcePos;

	assert(!sourcePosLookup->type);
	sourcePosLookup->type = SOURCE_TYPE_THREAD_START;

	scrParserGlob.threadStartSourceIndex = -1;
}

/*
==============
RemoveOpcodePos
==============
*/
void RemoveOpcodePos()
{
	OpcodeLookup *opcodeLookup;

	if ( !scrVarPub.developer )
	{
		return;
	}

	if ( scrCompilePub.developer_statement == SCR_DEV_IGNORE )
	{
		assert(!scrVarPub.developer_script);
		return;
	}

	assert(scrParserGlob.opcodeLookup);
	assert(scrParserGlob.opcodeLookupMaxLen);
	assert(scrParserGlob.sourcePosLookup);
	assert(scrCompilePub.opcodePos);

	assert(scrParserGlob.sourcePosLookupLen);
	scrParserGlob.sourcePosLookupLen--;

	assert(scrParserGlob.opcodeLookupLen);
	scrParserGlob.opcodeLookupLen--;

	assert(scrParserGlob.currentSourcePosCount);
	scrParserGlob.currentSourcePosCount--;

	opcodeLookup = &scrParserGlob.opcodeLookup[scrParserGlob.opcodeLookupLen];

	assert(scrParserGlob.currentCodePos == scrCompilePub.opcodePos);
	assert(opcodeLookup->sourcePosIndex + scrParserGlob.currentSourcePosCount == scrParserGlob.sourcePosLookupLen);
	assert(opcodeLookup->codePos == (char *)scrParserGlob.currentCodePos);

	if ( !scrParserGlob.currentSourcePosCount )
	{
		scrParserGlob.currentCodePos = NULL;
	}

	opcodeLookup->sourcePosCount = scrParserGlob.currentSourcePosCount;
}

/*
==============
Scr_ShutdownOpcodeLookup
==============
*/
void Scr_ShutdownOpcodeLookup()
{
	int i;

	if ( scrParserGlob.opcodeLookup )
	{
		Z_Free( scrParserGlob.opcodeLookup );
		scrParserGlob.opcodeLookup = NULL;
	}

	if ( scrParserGlob.sourcePosLookup )
	{
		Z_Free( scrParserGlob.sourcePosLookup );
		scrParserGlob.sourcePosLookup = NULL;
	}

	if ( scrParserPub.sourceBufferLookup )
	{
		for ( i = 0; i < scrParserPub.sourceBufferLookupLen; i++ )
		{
			Z_Free( scrParserPub.sourceBufferLookup[i].buf );
		}

		Z_Free( scrParserPub.sourceBufferLookup );
		scrParserPub.sourceBufferLookup = NULL;
	}

	if ( scrParserGlob.saveSourceBufferLookup )
	{
		for ( i = 0; i < scrParserGlob.saveSourceBufferLookupLen; i++ )
		{
			if ( scrParserGlob.saveSourceBufferLookup[i].sourceBuf )
			{
				Z_Free( scrParserGlob.saveSourceBufferLookup[i].sourceBuf );
			}
		}

		Z_Free( scrParserGlob.saveSourceBufferLookup );
		scrParserGlob.saveSourceBufferLookup = NULL;
	}
}

/*
==============
Scr_GetLineNum
==============
*/
unsigned int Scr_GetLineNum( unsigned int bufferIndex, unsigned int sourcePos )
{
	const char *startLine;
	int col;

	assert( scrVarPub.developer );
	return Scr_GetLineNumInternal( scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, sourcePos, &startLine, &col );
}

/*
==============
Scr_GetPrevSourcePos
==============
*/
unsigned int Scr_GetPrevSourcePos( const char *codePos, unsigned int index )
{
	return scrParserGlob.sourcePosLookup[ Scr_GetPrevSourcePosOpcodeLookup( codePos )->sourcePosIndex + index ].sourcePos;
}

/*
==============
Scr_GetFileAndLine
==============
*/
void Scr_GetFileAndLine( const char *codePos, char **filename, int *linenum )
{
	OpcodeLookup *opcodeLookup;
	unsigned int bufferIndex, sourcePos;

	assert( Scr_IsInScriptMemory( codePos ) );
	opcodeLookup = Scr_GetPrevSourcePosOpcodeLookup(codePos);

	if ( !opcodeLookup )
	{
		*linenum = 0;
		*filename = "";
		return;
	}

	sourcePos = scrParserGlob.sourcePosLookup[opcodeLookup->sourcePosIndex].sourcePos;
	bufferIndex = Scr_GetSourceBuffer(codePos);

	*linenum = Scr_GetLineNum(bufferIndex, sourcePos) + 1;
	*filename = scrParserPub.sourceBufferLookup[bufferIndex].buf;
}

/*
==============
Scr_GetSourcePos
==============
*/
int Scr_GetSourcePos( unsigned int bufferIndex, unsigned int sourcePos, char *outBuf, int outBufLen )
{
	int col, lineNum;
	char line[MAX_STRING_CHARS];

	assert(scrVarPub.developer);
	lineNum = Scr_GetLineInfo(scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, sourcePos, &col, line);

	if ( scrParserGlob.saveSourceBufferLookup )
		Com_sprintf( outBuf, outBufLen, "%s // %s%s, line %d", line, scrParserPub.sourceBufferLookup[bufferIndex].buf, " (savegame)", lineNum + 1);
	else
		Com_sprintf( outBuf, outBufLen, "%s // %s%s, line %d", line, scrParserPub.sourceBufferLookup[bufferIndex].buf, "", lineNum + 1);

	return lineNum;
}

/*
==============
Scr_GetSourcePosOfType
==============
*/
int Scr_GetSourcePosOfType( const char *codePos, int type, Scr_SourcePos_t *pos )
{
	OpcodeLookup *opcodeLookup;
	unsigned int index;

	opcodeLookup = Scr_GetSourcePosOpcodeLookup( codePos );

	if ( !opcodeLookup )
	{
		return 0;
	}

	for ( index = 0; index < opcodeLookup->sourcePosCount; index++ )
	{
		if ( ( scrParserGlob.sourcePosLookup[opcodeLookup->sourcePosIndex + index].type & type ) != type )
		{
			continue;
		}

		pos->sourcePos = scrParserGlob.sourcePosLookup[opcodeLookup->sourcePosIndex + index].sourcePos;
		pos->bufferIndex = Scr_GetSourceBuffer(codePos);
		pos->lineNum = Scr_GetLineNum(pos->bufferIndex, pos->sourcePos);

		return 1;
	}

	return 0;
}

/*
==============
AddOpcodePos
==============
*/
void AddOpcodePos( unsigned int sourcePos, int type )
{
	SourceLookup *sourcePosLookup, *newSourcePosLookup;
	OpcodeLookup *opcodeLookup, *newOpcodeLookup;
	int sourcePosLookupIndex;

	if ( !scrVarPub.developer )
	{
		return;
	}

	if ( scrCompilePub.developer_statement == SCR_DEV_IGNORE )
	{
		assert(!scrVarPub.developer_script);
		return;
	}

	if ( !scrCompilePub.allowedBreakpoint )
	{
		type &= ~SOURCE_TYPE_BREAKPOINT;
	}

	assert(scrParserGlob.opcodeLookup);
	assert(scrParserGlob.opcodeLookupMaxLen);
	assert(scrParserGlob.sourcePosLookup);
	assert(scrCompilePub.opcodePos);

	if ( scrParserGlob.opcodeLookupLen >= scrParserGlob.opcodeLookupMaxLen )
	{
		scrParserGlob.opcodeLookupMaxLen *= 2;
		assert(scrParserGlob.opcodeLookupLen < scrParserGlob.opcodeLookupMaxLen);

		newOpcodeLookup = (OpcodeLookup *)Z_Malloc(sizeof(*newOpcodeLookup) * scrParserGlob.opcodeLookupMaxLen);
		memcpy(newOpcodeLookup, scrParserGlob.opcodeLookup, sizeof(*newOpcodeLookup) * scrParserGlob.opcodeLookupLen);

		Z_Free(scrParserGlob.opcodeLookup);
		scrParserGlob.opcodeLookup = newOpcodeLookup;
	}

	if ( scrParserGlob.sourcePosLookupLen >= scrParserGlob.sourcePosLookupMaxLen )
	{
		scrParserGlob.sourcePosLookupMaxLen *= 2;
		assert(scrParserGlob.sourcePosLookupLen < scrParserGlob.sourcePosLookupMaxLen);

		newSourcePosLookup = (SourceLookup *)Z_Malloc(sizeof(*newSourcePosLookup) * scrParserGlob.sourcePosLookupMaxLen);
		memcpy(newSourcePosLookup, scrParserGlob.sourcePosLookup, sizeof(*newSourcePosLookup) * scrParserGlob.sourcePosLookupLen);

		Z_Free(scrParserGlob.sourcePosLookup);
		scrParserGlob.sourcePosLookup = newSourcePosLookup;
	}

	if ( scrParserGlob.currentCodePos == scrCompilePub.opcodePos )
	{
		assert(scrParserGlob.currentSourcePosCount);
		--scrParserGlob.opcodeLookupLen;
		opcodeLookup = &scrParserGlob.opcodeLookup[scrParserGlob.opcodeLookupLen];
		assert(opcodeLookup->sourcePosIndex + scrParserGlob.currentSourcePosCount == scrParserGlob.sourcePosLookupLen);
		assert(opcodeLookup->codePos == (char *)scrParserGlob.currentCodePos);
	}
	else
	{
		scrParserGlob.currentSourcePosCount = 0;
		scrParserGlob.currentCodePos = scrCompilePub.opcodePos;

		opcodeLookup = &scrParserGlob.opcodeLookup[scrParserGlob.opcodeLookupLen];
		opcodeLookup->sourcePosIndex = scrParserGlob.sourcePosLookupLen;
		opcodeLookup->codePos = (const char *)scrParserGlob.currentCodePos;
	}

	sourcePosLookupIndex = opcodeLookup->sourcePosIndex + scrParserGlob.currentSourcePosCount;
	sourcePosLookup = &scrParserGlob.sourcePosLookup[sourcePosLookupIndex];
	sourcePosLookup->sourcePos = sourcePos;

	if ( sourcePos == -1 )
	{
		assert(scrParserGlob.delayedSourceIndex == -1);
		assert(type & SOURCE_TYPE_BREAKPOINT);
		scrParserGlob.delayedSourceIndex = sourcePosLookupIndex;
	}
	else if ( sourcePos == -2 )
	{
		scrParserGlob.threadStartSourceIndex = sourcePosLookupIndex;
	}
	else if ( scrParserGlob.delayedSourceIndex >= 0 && type & SOURCE_TYPE_BREAKPOINT )
	{
		scrParserGlob.sourcePosLookup[scrParserGlob.delayedSourceIndex].sourcePos = sourcePos;
		scrParserGlob.delayedSourceIndex = -1;
	}

	sourcePosLookup->type |= type;

	scrParserGlob.currentSourcePosCount++;
	opcodeLookup->sourcePosCount = scrParserGlob.currentSourcePosCount;

	scrParserGlob.opcodeLookupLen++;
	scrParserGlob.sourcePosLookupLen++;
}

/*
==============
Scr_InitOpcodeLookup
==============
*/
void Scr_InitOpcodeLookup()
{
	assert(!scrParserGlob.opcodeLookup);
	assert(!scrParserGlob.sourcePosLookup);
	assert(!scrParserPub.sourceBufferLookup);

	if ( !scrVarPub.developer )
	{
		return;
	}

	scrParserGlob.delayedSourceIndex = -1;

	scrParserGlob.opcodeLookupMaxLen = INITIAL_OPCODE_LOOKUP_LEN;
	scrParserGlob.opcodeLookupLen = 0;
	scrParserGlob.opcodeLookup = (OpcodeLookup *)Z_Malloc( sizeof( *scrParserGlob.opcodeLookup ) * INITIAL_OPCODE_LOOKUP_LEN );
	memset(scrParserGlob.opcodeLookup, 0, sizeof( *scrParserGlob.opcodeLookup ) * scrParserGlob.opcodeLookupMaxLen);

	scrParserGlob.sourcePosLookupMaxLen = INITIAL_SOURCEPOS_LOOKUP_LEN;
	scrParserGlob.sourcePosLookupLen = 0;
	scrParserGlob.sourcePosLookup = (SourceLookup *)Z_Malloc( sizeof( *scrParserGlob.sourcePosLookup ) * INITIAL_SOURCEPOS_LOOKUP_LEN );

	scrParserGlob.currentCodePos = NULL;

	scrParserGlob.currentSourcePosCount = 0;
	scrParserGlob.sourceBufferLookupMaxLen = INITIAL_SOURCEBUFFER_LOOKUP_LEN;

	scrParserPub.sourceBufferLookupLen = 0;
	scrParserPub.sourceBufferLookup = (SourceBufferInfo *)Z_Malloc( sizeof( *scrParserPub.sourceBufferLookup ) * INITIAL_SOURCEBUFFER_LOOKUP_LEN );
}

/*
==============
Scr_GetCodePos
==============
*/
void Scr_GetCodePos( const char *codePos, unsigned int index, char *outBuf, int outBufLen )
{
	Scr_SourcePos_t pos;

	if ( !scrVarPub.developer )
	{
		assert( Scr_IsInScriptMemory( codePos ) );
		Com_sprintf(outBuf, outBufLen, "@ %d", codePos - scrVarPub.programBuffer);
		return;
	}

	Scr_GetSourcePosOfType(codePos, SOURCE_TYPE_THREAD_START, &pos);
	Scr_GetSourcePos(pos.bufferIndex, pos.sourcePos, outBuf, outBufLen);
}

/*
==============
Scr_PrintPrevCodePos
==============
*/
void Scr_PrintPrevCodePos( conChannel_t channel, const char *codePos, unsigned int index )
{
	unsigned int bufferIndex;

	if ( !codePos )
	{
		Com_PrintMessage(channel, "<frozen thread>\n");
		return;
	}

	if ( codePos == &g_EndPos )
	{
		Com_PrintMessage(channel, "<removed thread>\n");
		return;
	}

	if ( scrVarPub.developer )
	{
		if ( scrVarPub.programBuffer )
		{
			if ( Scr_IsInScriptMemory(codePos) )
			{
				bufferIndex = Scr_GetSourceBuffer( codePos - 1 );
				Scr_PrintSourcePos( channel, scrParserPub.sourceBufferLookup[bufferIndex].buf, scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, Scr_GetPrevSourcePos( codePos - 1, index ) );
				return;
			}
		}
	}
	else
	{
		if ( Scr_IsInScriptMemory( codePos - 1 ) )
		{
			Com_PrintMessage(channel, va("@ %d\n", codePos - scrVarPub.programBuffer));
			return;
		}
	}

	Com_PrintMessage( channel, va("%s\n\n", codePos) );
}

/*
==============
Scr_AddSourceBuffer
==============
*/
char* Scr_AddSourceBuffer( const char *filename, const char *extFilename, const char *codePos, bool archive )
{
	char eol, c;
	SaveSourceBufferInfo *saveSourceBuffer;
	char *source, *dest, *sourceBuf;
	int len, i;

	if ( !archive || !scrParserGlob.saveSourceBufferLookup )
	{
		return Scr_ReadFile( filename, extFilename, codePos, archive );
	}

	assert(scrParserGlob.saveSourceBufferLookupLen > 0);
	--scrParserGlob.saveSourceBufferLookupLen;
	saveSourceBuffer = &scrParserGlob.saveSourceBufferLookup[scrParserGlob.saveSourceBufferLookupLen];

	len = saveSourceBuffer->len;
	assert(len >= -1);

	if ( len < 0 )
	{
		Scr_AddSourceBufferInternal( extFilename, codePos, NULL, len, true, archive );
		return NULL;
	}

	sourceBuf = (char *)Hunk_AllocateTempMemoryHighInternal( len + 1 );
	source = saveSourceBuffer->sourceBuf;
	dest = sourceBuf;

	for ( i = 0; i < len; i++ )
	{
		c = *source++;
		eol = c;

		if ( !c )
		{
			eol = 10;
		}

		*dest++ = eol;
	}

	*dest = 0;

	if ( saveSourceBuffer->sourceBuf )
	{
		Z_Free( scrParserGlob.saveSourceBufferLookup[scrParserGlob.saveSourceBufferLookupLen].sourceBuf );
	}

	Scr_AddSourceBufferInternal( extFilename, codePos, sourceBuf, len, true, archive );
	return sourceBuf;
}

/*
==============
RuntimeError
==============
*/
void RuntimeError( const char *codePos, unsigned int index, const char *msg, const char *dialogMessage )
{
	bool abort_on_error;

	if ( !scrVarPub.developer )
	{
		assert( Scr_IsInScriptMemory( codePos ) );

		if ( !scrVmPub.terminal_error )
		{
#ifdef LIBCOD
			// libcod: logErrors logs non-terminal script errors to the logfile even when
			// developer mode is off. Safe: when !developer, Scr_PrintPrevCodePos prints only a
			// bytecode offset, not the developer-only source lookup. Skipped while loading.
			if ( logErrors && logErrors->current.boolean && !scrVmGlob.loading )
				RuntimeErrorInternal(CON_CHANNEL_LOGFILEONLY, codePos, index, msg);
#endif
			return;
		}
	}

	if ( scrVmPub.debugCode )
	{
		Com_Printf("%s\n", msg);

		if ( !scrVmPub.terminal_error )
		{
			return;
		}

		if ( dialogMessage )
		{
			if ( scrVmPub.terminal_error )
				Com_Error(ERR_SCRIPT_DROP, "script runtime error\n(see console for details)\n", msg, "\n", dialogMessage);
			else
				Com_Error(ERR_SCRIPT, "script runtime error\n(see console for details)\n", msg, "\n", dialogMessage);
		}
		else
		{
			if ( scrVmPub.terminal_error )
				Com_Error(ERR_SCRIPT_DROP, "script runtime error\n(see console for details)\n", msg, "", "");
			else
				Com_Error(ERR_SCRIPT, "script runtime error\n(see console for details)\n", msg, "", "");
		}
	}
	else
	{
		abort_on_error = false;

		if ( scrVmPub.abort_on_error || scrVmPub.terminal_error )
		{
			abort_on_error = true;
		}

		if ( abort_on_error )
			RuntimeErrorInternal(CON_CHANNEL_DONT_FILTER, codePos, index, msg);
		else
			RuntimeErrorInternal(CON_CHANNEL_LOGFILEONLY, codePos, index, msg);

#ifdef LIBCOD
		if ( com_developer->current.integer == 2 )
			abort_on_error = false;
#endif

		if ( !abort_on_error )
		{
			return;
		}

		if ( dialogMessage )
		{
			if ( scrVmPub.terminal_error )
				Com_Error(ERR_SCRIPT_DROP, "script runtime error\n(see console for details)\n", msg, "\n", dialogMessage);
			else
				Com_Error(ERR_SCRIPT, "script runtime error\n(see console for details)\n", msg, "\n", dialogMessage);
		}
		else
		{
			if ( scrVmPub.terminal_error )
				Com_Error(ERR_SCRIPT_DROP, "script runtime error\n(see console for details)\n", msg, "", "");
			else
				Com_Error(ERR_SCRIPT, "script runtime error\n(see console for details)\n", msg, "", "");
		}
	}
}

/*
==============
CompileError
==============
*/
void CompileError( unsigned int sourcePos, const char *format, ... )
{
	char text[MAX_STRING_CHARS];
	va_list argptr;

	va_start( argptr, format );
	Q_vsnprintf( text, sizeof( text ), format, argptr );
	va_end( argptr );

	if ( !scrVarPub.evaluate )
	{
		Com_Printf("\n");
		Com_Printf("******* script compile error *******\n");

		if ( scrVarPub.developer )
		{
			Com_Printf("%s: ", text);
			Scr_PrintSourcePos(CON_CHANNEL_DONT_FILTER, scrParserPub.scriptfilename, scrParserPub.sourceBuf, sourcePos);
		}
		else
		{
			Com_Printf("%s\n", text);
		}

		Com_Printf("************************************\n");
		Com_Error(ERR_SCRIPT_DROP, "script compile error\n(see console for details)\n");
	}

	if ( !scrVarPub.error_message )
	{
		scrVarPub.error_message = va("%s", text);
	}
}

/*
==============
CompileError2
==============
*/
void CompileError2( const char *codePos, const char *msg, ... )
{
	char text[MAX_STRING_CHARS];
	va_list argptr;

	assert( !scrVarPub.evaluate );
	assert( Scr_IsInScriptMemory( codePos ) );

	Com_Printf("\n");
	Com_Printf("******* script compile error *******\n");

	va_start( argptr, msg );
	Q_vsnprintf( text, sizeof( text ), msg, argptr );
	va_end( argptr );

	Com_Printf("%s: ", text);
	Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, codePos, 0);

	Com_Printf("************************************\n");
	Com_Error(ERR_SCRIPT_DROP, "script compile error\n(see console for details)\n");
}

/*
==============
Scr_GetLineNumInternal
==============
*/
unsigned int Scr_GetLineNumInternal( const char *buf, unsigned int sourcePos, const char **startLine, int *col )
{
	unsigned int lineNum;

	assert(buf);
	*startLine = buf;
	lineNum = 0;

	while ( sourcePos )
	{
		if ( !buf[0] )
		{
			*startLine = buf + 1;
			lineNum++;
		}

		buf++;
		sourcePos--;
	}

	*col = buf - *startLine;
	return lineNum;
}

/*
==============
Scr_GetSourcePosOpcodeLookup
==============
*/
OpcodeLookup* Scr_GetSourcePosOpcodeLookup( const char *codePos )
{
	int low, middle, high;

	assert( Scr_IsInScriptMemory( codePos ) );
	assert( scrParserGlob.opcodeLookup );

	low = 0;
	high = scrParserGlob.opcodeLookupLen - 1;

	while ( low <= high )
	{
		middle = ( high + low ) / 2;

		if ( codePos < scrParserGlob.opcodeLookup[middle].codePos )
		{
			high = middle - 1;
			continue;
		}

		if ( codePos == scrParserGlob.opcodeLookup[middle].codePos )
		{
			return &scrParserGlob.opcodeLookup[middle];
		}

		low = middle + 1;
	}

	return NULL;
}

/*
==============
Scr_GetPrevSourcePosOpcodeLookup
==============
*/
OpcodeLookup* Scr_GetPrevSourcePosOpcodeLookup( const char *codePos )
{
	int low, middle, high;

	assert( Scr_IsInScriptMemory( codePos ) );
	assert( scrParserGlob.opcodeLookup );

	low = 0;
	high = scrParserGlob.opcodeLookupLen - 1;

	while ( low <= high )
	{
		middle = ( high + low ) / 2;

		if ( codePos < scrParserGlob.opcodeLookup[middle].codePos )
		{
			high = middle - 1;
			continue;
		}

		low = middle + 1;

		if ( low == scrParserGlob.opcodeLookupLen || codePos < scrParserGlob.opcodeLookup[low].codePos )
		{
			return &scrParserGlob.opcodeLookup[middle];
		}
	}

	return NULL;
}

/*
==============
Scr_CopyFormattedLine
==============
*/
void Scr_CopyFormattedLine( char *line, const char *rawLine )
{
	int len, i;

	len = strlen(rawLine);

	if ( len >= MAX_STRING_CHARS )
	{
		len = MAX_STRING_CHARS - 1;
	}

	for ( i = 0; i < len; i++ )
	{
		if ( rawLine[i] == 9 )
			line[i] = ' ';
		else
			line[i] = rawLine[i];
	}

	if ( line[len - 1] == 13 )
	{
		line[len - 1] = 0;
	}

	line[len] = 0;
}

/*
==============
Scr_GetLineInfo
==============
*/
unsigned int Scr_GetLineInfo( const char *buf, unsigned int sourcePos, int *col, char *line )
{
	const char *startLine;
	unsigned int lineNum;

	if ( buf )
	{
		lineNum = Scr_GetLineNumInternal(buf, sourcePos, &startLine, col);
	}
	else
	{
		lineNum = 0;
		startLine = "";
		*col = 0;
	}

	Scr_CopyFormattedLine(line, startLine);
	return lineNum;
}

/*
==============
Scr_PrintSourcePos
==============
*/
void Scr_PrintSourcePos( conChannel_t channel, const char *filename, const char *buf, unsigned int sourcePos )
{
	int i, col;
	unsigned int lineNum;
	char line[MAX_STRING_CHARS];

	assert(filename);
	lineNum = Scr_GetLineInfo(buf, sourcePos, &col, line);

	if ( scrParserGlob.saveSourceBufferLookup )
		Com_PrintMessage(channel, va("(file '%s'%s, line %d)\n", filename, " (savegame)", lineNum + 1));
	else
		Com_PrintMessage(channel, va("(file '%s'%s, line %d)\n", filename, "", lineNum + 1));

	Com_PrintMessage(channel, va("%s\n", line));

	for ( i = 0; i < col; i++ )
	{
		Com_PrintMessage(channel, " ");
	}

	Com_PrintMessage(channel, "*\n");
}

/*
==============
Scr_GetNewSourceBuffer
==============
*/
SourceBufferInfo* Scr_GetNewSourceBuffer()
{
	SourceBufferInfo *newSourceBufferInfo;

	assert(scrParserPub.sourceBufferLookup);
	assert(scrParserGlob.sourceBufferLookupMaxLen);

	if ( scrParserPub.sourceBufferLookupLen >= scrParserGlob.sourceBufferLookupMaxLen )
	{
		scrParserGlob.sourceBufferLookupMaxLen *= 2;
		assert(scrParserPub.sourceBufferLookupLen < scrParserGlob.sourceBufferLookupMaxLen);

		newSourceBufferInfo = (SourceBufferInfo *)Z_Malloc(sizeof(*newSourceBufferInfo) * scrParserGlob.sourceBufferLookupMaxLen);
		Com_Memcpy(newSourceBufferInfo, scrParserPub.sourceBufferLookup, sizeof(*newSourceBufferInfo) * scrParserPub.sourceBufferLookupLen);

		Z_Free(scrParserPub.sourceBufferLookup);
		scrParserPub.sourceBufferLookup = newSourceBufferInfo;
	}

	return &scrParserPub.sourceBufferLookup[scrParserPub.sourceBufferLookupLen++];
}

/*
==============
Scr_AddSourceBufferInternal
==============
*/
void Scr_AddSourceBufferInternal( const char *extFilename, const char *codePos, char *sourceBuf, int len, bool doEolFixup, bool archive )
{
	SourceBufferInfo *newSourceBuffer;
	char *source, *dest, *buf, *b;
	int i, l, c;

	if ( !scrParserPub.sourceBufferLookup )
	{
		scrParserPub.sourceBuf = NULL;
		return;
	}

	l = strlen( extFilename ) + 1;
	buf = (char *)Z_Malloc( l + len + 2 );
	strcpy( buf, extFilename );

	if ( sourceBuf )
		b = &buf[l];
	else
		b = NULL;

	source = sourceBuf;
	dest = b;

	if ( doEolFixup )
	{
		for ( i = 0; i <= len; i++ )
		{
			c = *source++;

			if ( c == 10 || c == 13 && *source != 10 )
				*dest = 0;
			else
				*dest = c;

			dest++;
		}
	}
	else
	{
		for ( i = 0; i <= len; i++ )
		{
			*dest++ = *source++;
		}
	}

	newSourceBuffer = Scr_GetNewSourceBuffer();

	newSourceBuffer->codePos = codePos;
	newSourceBuffer->buf = buf;
	newSourceBuffer->sourceBuf = b;
	newSourceBuffer->len = len;
	newSourceBuffer->sortedIndex = -1;
	newSourceBuffer->archive = archive;

	if ( b )
	{
		scrParserPub.sourceBuf = b;
	}
}

/*
==============
Scr_ReadFile
==============
*/
char* Scr_ReadFile( const char *filename, const char *extFilename, const char *codePos, bool archive )
{
	fileHandle_t file;
	int len;
	char *buf;

	len = FS_FOpenFileByMode( extFilename, &file, FS_READ );

	if ( len < 0 )
	{
		Scr_AddSourceBufferInternal( extFilename, codePos, NULL, -1, true, archive );
		return NULL;
	}

	buf = (char *)Hunk_AllocateTempMemoryHighInternal( len + 1 );

	FS_Read( buf, len, file );
	buf[len] = 0;

	FS_FCloseFile( file );
	Scr_AddSourceBufferInternal( extFilename, codePos, buf, len, true, archive );

	return buf;

}

/*
==============
RuntimeErrorInternal
==============
*/
void RuntimeErrorInternal( conChannel_t channel, const char *codePos, unsigned int index, const char *msg )
{
	int i;

	assert( Scr_IsInScriptMemory( codePos ) );
	Com_PrintMessage(channel, va("\n******* script runtime error *******\n%s: ", msg));
	Scr_PrintPrevCodePos(channel, codePos, index);

	if ( scrVmPub.function_count )
	{
		for ( i = scrVmPub.function_count - 1; i > 0; i-- )
		{
			Com_PrintMessage(channel, "called from:\n");
			Scr_PrintPrevCodePos(channel, scrVmPub.function_frame_start[i].fs.pos, scrVmPub.function_frame_start[i].fs.localId == 0);
		}

		Com_PrintMessage(channel, "started from:\n");
		Scr_PrintPrevCodePos(channel, scrVmPub.function_frame_start[0].fs.pos, 1);
	}

	Com_PrintMessage(channel, "************************************\n");
}
