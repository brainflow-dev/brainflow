#!/usr/bin/env python3
import argparse
import json
import os
import urllib.parse
import urllib.request

API_BASE_URL = "https://api.github.com"
REQUEST_ACCEPT_VERSION = "application/vnd.github.v3+json"
REQUEST_USER_AGENT = "magnetikonline/remove-workflow-run"


def github_request(
    auth_token, path, method=None, parameter_collection=None, parse_response=True
):
    # build base request URL/headers
    request_url = f"{API_BASE_URL}/{path}"
    header_collection = {
        "Accept": REQUEST_ACCEPT_VERSION,
        "Authorization": f"token {auth_token}",
        "User-Agent": REQUEST_USER_AGENT,
    }

    if method is None:
        # GET method
        if parameter_collection is not None:
            request_url = (
                f"{request_url}?{urllib.parse.urlencode(parameter_collection)}"
            )

        request = urllib.request.Request(headers=header_collection, url=request_url)
    else:
        # POST/PATCH/PUT/DELETE method
        request = urllib.request.Request(
            headers=header_collection, method=method, url=request_url
        )

    response = urllib.request.urlopen(request)
    response_data = {}
    if parse_response:
        response_data = json.load(response)

    response.close()

    return response_data


def workflow_run_list(auth_token, owner_repo_name, workflow_id):
    request_page = 1
    while True:
        data = github_request(
            auth_token,
            f"repos/{owner_repo_name}/actions/workflows/{urllib.parse.quote(workflow_id)}/runs",
            parameter_collection={"page": request_page},
        )

        run_list = data["workflow_runs"]
        if len(run_list) < 1:
            # no more items
            break

        for item in run_list:
            print(f"Found run ID: {item['id']}")
            yield item["id"]

        # move to next page
        request_page += 1


def workflow_run_delete(auth_token, owner_repo_name, run_id):
    github_request(
        auth_token,
        f"repos/{owner_repo_name}/actions/runs/{run_id}",
        method="DELETE",
        parse_response=False,
    )


def main():
    # fetch GitHub access token
    auth_token = os.environ["AUTH_TOKEN"]

    # fetch requested repository and workflow ID to remove prior runs from
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-name", required=True)
    parser.add_argument("--workflow-id", required=True)
    arg_list = parser.parse_args()

    # fetch run id list from repository workflow
    run_id_list = list(
        workflow_run_list(auth_token, arg_list.repository_name, arg_list.workflow_id)
    )

    for run_id in run_id_list:
        print(f"Deleting run ID: {run_id}")
        workflow_run_delete(auth_token, arg_list.repository_name, run_id)


if __name__ == "__main__":
    main()