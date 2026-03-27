import express from "express";
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";
import { spawn } from "child_process";

type SynDict = Record<string, string[]>;

type ImproveResponse = {
  original: string;
  improved: string;
  repetitions: { word: string; count: number; replacements: string[] }[];
};

type MorphAnalysis = {
  lemma: string;
  cls: number;
};

type TokenInfo = {
  raw: string;
  lead: string;
  core: string;
  trail: string;
  normalized: string;
  lookupKey: string | null;
  cls: number;
};

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
app.use(express.json({ limit: "2mb" }));
app.use(express.static(path.join(__dirname, "public")));

const SYN_PATH = path.join(__dirname, "synonyms.json");
const CPP_EXE = path.join(__dirname, "infinite.exe");
const RECONSTRUCT_EXE = path.join(__dirname, "reconstruct.exe");
const THRESHOLD = 2;

function loadSynonyms(): SynDict {
  const raw = fs.readFileSync(SYN_PATH, "utf8");
  const parsed = JSON.parse(raw) as SynDict;
  const out: SynDict = {};

  for (const [k, v] of Object.entries(parsed)) {
    out[k.toLowerCase()] = (v ?? []).map(String);
  }

  return out;
}

let SYNONYMS: SynDict = loadSynonyms();

const PUNCT_RE = /^[\s"'“‘(\[{]+|[\s"'”’)\]}.,;:!?]+$/g;

function normalizeToken(token: string): string {
  return token.toLowerCase().replace(PUNCT_RE, "");
}

function stripEnds(token: string): { lead: string; core: string; trail: string } {
  const m = token.match(/^([\s"'“‘(\[{]*)(.*?)([\s"'”’)\]}.,;:!?]*)$/);
  if (!m) {
    return { lead: "", core: token, trail: "" };
  }

  return {
    lead: m[1] ?? "",
    core: m[2] ?? token,
    trail: m[3] ?? ""
  };
}

function replacePreserveCase(originalCore: string, replacement: string): string {
  if (!originalCore || !replacement) return replacement;

  const first = originalCore[0];
  if (first === first.toUpperCase() && first !== first.toLowerCase()) {
    return replacement[0].toUpperCase() + replacement.slice(1);
  }

  return replacement;
}

function parseAnalysisOutput(output: string): MorphAnalysis | null {
  const lines = output
    .split(/\r?\n/)
    .map(s => s.trim())
    .filter(Boolean);

  if (lines.length < 2) return null;

  const lemma = normalizeToken(lines[0]);
  const cls = Number(lines[1]);

  if (!lemma || Number.isNaN(cls)) return null;

  return { lemma, cls };
}

function askAnalysisFromCpp(word: string): Promise<MorphAnalysis | null> {
  return new Promise((resolve, reject) => {
    const child = spawn(CPP_EXE, [], { stdio: ["pipe", "pipe", "pipe"] });

    let stdout = "";
    let stderr = "";

    child.stdout.on("data", chunk => {
      stdout += chunk.toString();
    });

    child.stderr.on("data", chunk => {
      stderr += chunk.toString();
    });

    child.on("error", err => {
      reject(err);
    });

    child.on("close", code => {
      console.log("SENDING TO INFINITE =", word);
      console.log("INFINITE RAW STDOUT =", JSON.stringify(stdout));
      console.log("INFINITE RAW STDERR =", JSON.stringify(stderr));

      if (code !== 0) {
        reject(new Error(`infinite.exe exited with code ${code}: ${stderr}`));
        return;
      }

      const parsed = parseAnalysisOutput(stdout);
      console.log("INFINITE PARSED =", parsed);

      resolve(parsed);
    });

    child.stdin.write(word + "\n");
    child.stdin.end();
  });
}

function askReconstructFromCpp(lemmaOrPhrase: string, cls: number): Promise<string> {
  return new Promise((resolve, reject) => {
    const child = spawn(RECONSTRUCT_EXE, [], { stdio: ["pipe", "pipe", "pipe"] });

    let stdout = "";
    let stderr = "";

    child.stdout.on("data", chunk => {
      stdout += chunk.toString();
    });

    child.stderr.on("data", chunk => {
      stderr += chunk.toString();
    });

    child.on("error", err => {
      reject(err);
    });

    child.on("close", code => {
      if (code !== 0) {
        reject(new Error(`reconstruct.exe exited with code ${code}: ${stderr}`));
        return;
      }

      resolve(stdout.trim());
    });

    child.stdin.write(`${lemmaOrPhrase}\n${cls}\n`);
    child.stdin.end();
  });
}

const analysisCache = new Map<string, MorphAnalysis | null>();
const synonymCache = new Map<string, string[] | null>();
const reconstructCache = new Map<string, string>();

async function resolveAnalysis(word: string): Promise<MorphAnalysis | null> {
  const key = normalizeToken(word);
  if (!key) return null;

  if (analysisCache.has(key)) {
    return analysisCache.get(key) ?? null;
  }

  if (SYNONYMS[key]?.length) {
    const direct = { lemma: key, cls: -1 };
    analysisCache.set(key, direct);
    return direct;
  }

  try {
    const analysis = await askAnalysisFromCpp(key);

    if (analysis?.lemma) {
      analysisCache.set(key, analysis);
      return analysis;
    }

    const fallback = { lemma: key, cls: -1 };
    analysisCache.set(key, fallback);
    return fallback;
  } catch (err) {
    console.error("C++ analysis failed for word:", key, err);

    const fallback = { lemma: key, cls: -1 };
    analysisCache.set(key, fallback);
    return fallback;
  }
}

async function findSynonymsForLookupKey(lookupKey: string): Promise<string[] | null> {
  if (!lookupKey) return null;

  if (synonymCache.has(lookupKey)) {
    return synonymCache.get(lookupKey) ?? null;
  }

  if (SYNONYMS[lookupKey]?.length) {
    synonymCache.set(lookupKey, SYNONYMS[lookupKey]);
    return SYNONYMS[lookupKey];
  }

  synonymCache.set(lookupKey, null);
  return null;
}

async function reconstructSynonymIfNeeded(synonym: string, cls: number): Promise<string> {
  if (cls === -1) return synonym;

  const cacheKey = `${synonym}||${cls}`;
  if (reconstructCache.has(cacheKey)) {
    return reconstructCache.get(cacheKey)!;
  }

  try {
    const reconstructed = await askReconstructFromCpp(synonym, cls);
    const result = reconstructed || synonym;
    reconstructCache.set(cacheKey, result);
    return result;
  } catch (err) {
    console.error("C++ reconstruction failed for synonym:", synonym, "class:", cls, err);
    reconstructCache.set(cacheKey, synonym);
    return synonym;
  }
}

async function improveText(text: string): Promise<ImproveResponse> {
  const parts = text.split(/(\s+)/);
  const tokenInfo: TokenInfo[] = [];

  for (const p of parts) {
    if (!p.trim() || /^\s+$/.test(p)) {
      tokenInfo.push({
        raw: p,
        lead: "",
        core: "",
        trail: "",
        normalized: "",
        lookupKey: null,
        cls: -1
      });
      continue;
    }

    const { lead, core, trail } = stripEnds(p);
    const normalized = normalizeToken(core);
    const analysis = normalized ? await resolveAnalysis(core) : null;

    tokenInfo.push({
      raw: p,
      lead,
      core,
      trail,
      normalized,
      lookupKey: analysis?.lemma ?? null,
      cls: analysis?.cls ?? -1
    });
  }

  const freq = new Map<string, number>();

  for (const t of tokenInfo) {
    if (!t.lookupKey) continue;
    freq.set(t.lookupKey, (freq.get(t.lookupKey) ?? 0) + 1);
  }

  const synonymMap = new Map<string, string[]>();

  for (const [lookupKey, count] of freq.entries()) {
    if (count < THRESHOLD) continue;

    const syns = await findSynonymsForLookupKey(lookupKey);
    if (syns && syns.length > 0) {
      synonymMap.set(lookupKey, syns);
    }
  }

  const repetitions: ImproveResponse["repetitions"] = [];

  for (const [lookupKey, count] of freq.entries()) {
    const syns = synonymMap.get(lookupKey);
    if (count >= THRESHOLD && syns && syns.length > 0) {
      repetitions.push({
        word: lookupKey,
        count,
        replacements: syns
      });
    }
  }

  repetitions.sort((a, b) => b.count - a.count);

  const seen = new Map<string, number>();
  let improved = "";

  for (const t of tokenInfo) {
    if (!t.lookupKey || !t.core) {
      improved += t.raw;
      continue;
    }

    const total = freq.get(t.lookupKey) ?? 0;
    const syns = synonymMap.get(t.lookupKey);

    if (!syns || syns.length === 0 || total < THRESHOLD) {
      improved += t.raw;
      continue;
    }

    const k = (seen.get(t.lookupKey) ?? 0) + 1;
    seen.set(t.lookupKey, k);

    if (k === 1) {
      improved += t.raw;
      continue;
    }

    const idx = (k - 2) % syns.length;
    const synonymLemma = syns[idx];
    const reconstructed = await reconstructSynonymIfNeeded(synonymLemma, t.cls);
    console.log("TOKEN:", t.core, "LEMMA:", t.lookupKey, "CLS:", t.cls);
    console.log("SYNONYM:", synonymLemma);
    console.log("RECONSTRUCTED:", reconstructed);
    const repl = replacePreserveCase(t.core, reconstructed);

    improved += `${t.lead}${repl}${t.trail}`;
    console.log("FINAL IMPROVED =", improved);
  }

  return {
    original: text,
    improved,
    repetitions
  };
}

app.post("/improve", async (req, res) => {
  try {
    const text = typeof req.body?.text === "string" ? req.body.text : "";

    if (!text.trim()) {
      return res.status(400).json({ error: "text is required" });
    }

    const result = await improveText(text);
    res.json(result);
  } catch (err) {
    console.error(err);
    res.status(500).json({ error: "internal server error" });
  }
});

app.listen(3000, () => {
  console.log("SERVER_TS_V99");
  console.log("http://localhost:3000");
});